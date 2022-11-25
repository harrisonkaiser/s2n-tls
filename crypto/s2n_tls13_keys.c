/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "crypto/s2n_tls13_keys.h"

#include <stdio.h>

#include "crypto/s2n_hkdf.h"
#include "crypto/s2n_hmac.h"
#include "error/s2n_errno.h"
#include "stuffer/s2n_stuffer.h"
#include "utils/s2n_blob.h"
#include "utils/s2n_mem.h"
#include "utils/s2n_safety.h"

/*
 * There are 9 keys that can be generated by the end of a TLS 1.3 handshake.
 * We currently support the following, more will be supported
 * when the relevant TLS 1.3 features are worked on.
 *
 * [x] binder_key
 * [x] client_early_traffic_secret
 * [ ] early_exporter_master_secret
 * [x] client_handshake_traffic_secret
 * [x] server_handshake_traffic_secret
 * [x] client_application_traffic_secret_0
 * [x] server_application_traffic_secret_0
 * [ ] exporter_master_secret
 * [x] resumption_master_secret
 *
 * The TLS 1.3 key generation can be divided into 3 phases
 * 1. early secrets
 * 2. handshake secrets
 * 3. master secrets
 *
 * In each phase, secrets are first extracted with HKDF-Extract that takes in
 * both an ikm (input keying material) and a salt. Some keys can be derived/expanded
 * from the extract before a "tls13 derived" Derive-Secret is used to
 * derive the input salt for the next phase.
 */

/*
 * Define TLS 1.3 HKDF labels as specified in
 * https://tools.ietf.org/html/rfc8446#section-7.1
 */
S2N_BLOB_LABEL(s2n_tls13_label_derived_secret, "derived")

S2N_BLOB_LABEL(s2n_tls13_label_external_psk_binder_key, "ext binder")
S2N_BLOB_LABEL(s2n_tls13_label_resumption_psk_binder_key, "res binder")

S2N_BLOB_LABEL(s2n_tls13_label_client_early_traffic_secret, "c e traffic")
S2N_BLOB_LABEL(s2n_tls13_label_early_exporter_master_secret, "e exp master")

S2N_BLOB_LABEL(s2n_tls13_label_client_handshake_traffic_secret, "c hs traffic")
S2N_BLOB_LABEL(s2n_tls13_label_server_handshake_traffic_secret, "s hs traffic")

S2N_BLOB_LABEL(s2n_tls13_label_client_application_traffic_secret, "c ap traffic")
S2N_BLOB_LABEL(s2n_tls13_label_server_application_traffic_secret, "s ap traffic")

S2N_BLOB_LABEL(s2n_tls13_label_exporter_master_secret, "exp master")
S2N_BLOB_LABEL(s2n_tls13_label_resumption_master_secret, "res master")
S2N_BLOB_LABEL(s2n_tls13_label_session_ticket_secret, "resumption")

/*
 * Traffic secret labels
 */
S2N_BLOB_LABEL(s2n_tls13_label_traffic_secret_key, "key")
S2N_BLOB_LABEL(s2n_tls13_label_traffic_secret_iv, "iv")

/*
 * TLS 1.3 Finished label
 */
S2N_BLOB_LABEL(s2n_tls13_label_finished, "finished")

/*
 * TLS 1.3 KeyUpdate label
 */
S2N_BLOB_LABEL(s2n_tls13_label_application_traffic_secret_update, "traffic upd")

static const struct s2n_blob zero_length_blob = { .data = NULL, .size = 0 };

/*
 * Initializes the tls13_keys struct
 */
int s2n_tls13_keys_init(struct s2n_tls13_keys *keys, s2n_hmac_algorithm alg)
{
    POSIX_ENSURE_REF(keys);

    keys->hmac_algorithm = alg;
    POSIX_GUARD(s2n_hmac_hash_alg(alg, &keys->hash_algorithm));
    POSIX_GUARD(s2n_hash_digest_size(keys->hash_algorithm, &keys->size));
    POSIX_GUARD(s2n_blob_init(&keys->extract_secret, keys->extract_secret_bytes, keys->size));
    POSIX_GUARD(s2n_blob_init(&keys->derive_secret, keys->derive_secret_bytes, keys->size));
    POSIX_GUARD(s2n_hmac_new(&keys->hmac));

    return 0;
}

/*
 * Frees any allocation
 */
int s2n_tls13_keys_free(struct s2n_tls13_keys *keys)
{
    POSIX_ENSURE_REF(keys);

    POSIX_GUARD(s2n_hmac_free(&keys->hmac));

    return 0;
}

/*
 * Derive Traffic Key and IV based on input secret
 */
int s2n_tls13_derive_traffic_keys(struct s2n_tls13_keys *keys, struct s2n_blob *secret,
        struct s2n_blob *key, struct s2n_blob *iv)
{
    POSIX_ENSURE_REF(keys);
    POSIX_ENSURE_REF(secret);
    POSIX_ENSURE_REF(key);
    POSIX_ENSURE_REF(iv);

    POSIX_GUARD(s2n_hkdf_expand_label(&keys->hmac, keys->hmac_algorithm, secret,
            &s2n_tls13_label_traffic_secret_key, &zero_length_blob, key));
    POSIX_GUARD(s2n_hkdf_expand_label(&keys->hmac, keys->hmac_algorithm, secret,
            &s2n_tls13_label_traffic_secret_iv, &zero_length_blob, iv));
    return 0;
}

/*
 * Generate finished key for compute finished hashes/MACs
 * https://tools.ietf.org/html/rfc8446#section-4.4.4
 */
int s2n_tls13_derive_finished_key(struct s2n_tls13_keys *keys, struct s2n_blob *secret_key,
        struct s2n_blob *output_finish_key)
{
    POSIX_GUARD(s2n_hkdf_expand_label(&keys->hmac, keys->hmac_algorithm, secret_key,
            &s2n_tls13_label_finished, &zero_length_blob, output_finish_key));

    return 0;
}

/*
 * Compute finished verify data using HMAC
 * with a finished key and hash state
 * https://tools.ietf.org/html/rfc8446#section-4.4.4
 */
int s2n_tls13_calculate_finished_mac(struct s2n_tls13_keys *keys, struct s2n_blob *finished_key,
        struct s2n_hash_state *hash_state, struct s2n_blob *finished_verify)
{
    s2n_tls13_key_blob(transcript_hash, keys->size);
    POSIX_GUARD(s2n_hash_digest(hash_state, transcript_hash.data, transcript_hash.size));
    POSIX_GUARD(s2n_hkdf_extract(&keys->hmac, keys->hmac_algorithm, finished_key, &transcript_hash, finished_verify));
    return S2N_SUCCESS;
}

/*
 * Derives next generation of traffic secret
 */
int s2n_tls13_update_application_traffic_secret(struct s2n_tls13_keys *keys, struct s2n_blob *old_secret,
        struct s2n_blob *new_secret)
{
    POSIX_ENSURE_REF(keys);
    POSIX_ENSURE_REF(old_secret);
    POSIX_ENSURE_REF(new_secret);

    POSIX_GUARD(s2n_hkdf_expand_label(&keys->hmac, keys->hmac_algorithm, old_secret,
            &s2n_tls13_label_application_traffic_secret_update, &zero_length_blob, new_secret));

    return 0;
}

S2N_RESULT s2n_tls13_derive_session_ticket_secret(struct s2n_tls13_keys *keys, struct s2n_blob *resumption_secret,
        struct s2n_blob *ticket_nonce, struct s2n_blob *secret_blob)
{
    RESULT_ENSURE_REF(keys);
    RESULT_ENSURE_REF(resumption_secret);
    RESULT_ENSURE_REF(ticket_nonce);
    RESULT_ENSURE_REF(secret_blob);

    /* Derive session ticket secret from master session resumption secret */
    RESULT_GUARD_POSIX(s2n_hkdf_expand_label(&keys->hmac, keys->hmac_algorithm, resumption_secret,
            &s2n_tls13_label_session_ticket_secret, ticket_nonce, secret_blob));

    return S2N_RESULT_OK;
}
