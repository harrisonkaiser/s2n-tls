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

/* Target Functions: s2n_server_finished_recv */

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <stdint.h>

#include "api/s2n.h"
#include "s2n_test.h"
#include "stuffer/s2n_stuffer.h"
#include "tls/s2n_connection.h"
#include "tls/s2n_tls.h"
#include "utils/s2n_safety.h"

static const uint8_t TLS_VERSIONS[] = { S2N_TLS10, S2N_TLS11, S2N_TLS12, S2N_TLS13, S2N_SSLv3 };

int s2n_fuzz_test(const uint8_t *buf, size_t len)
{
    /* We need at least one byte of input to set parameters */
    S2N_FUZZ_ENSURE_MIN_LEN(len, 1);

    /* Setup */
    struct s2n_connection *client_conn = s2n_connection_new(S2N_SERVER);
    POSIX_ENSURE_REF(client_conn);
    POSIX_GUARD(s2n_stuffer_write_bytes(&client_conn->handshake.io, buf, len));

    /* Pull a byte off the libfuzzer input and use it to set parameters */
    uint8_t randval = 0;
    POSIX_GUARD(s2n_stuffer_read_uint8(&client_conn->handshake.io, &randval));
    client_conn->actual_protocol_version = TLS_VERSIONS[randval % s2n_array_len(TLS_VERSIONS)];

    uint8_t finished_len = S2N_TLS_FINISHED_LEN;
    if (TLS_VERSIONS[randval % s2n_array_len(TLS_VERSIONS)] == S2N_SSLv3) {
        finished_len = S2N_SSL_FINISHED_LEN;
    }

    /* We do not use a GUARD macro here as there may not be enough bytes to write the necessary
     * amount, and the result of a failed read_bytes call is an acceptable test input. */
    s2n_stuffer_read_bytes(&client_conn->handshake.io, client_conn->handshake.server_finished, finished_len);

    /* Run Test
     * Do not use GUARD macro here since the connection memory hasn't been freed.
     */
    s2n_server_finished_recv(client_conn);

    /* Cleanup */
    POSIX_GUARD(s2n_connection_free(client_conn));

    return S2N_SUCCESS;
}

S2N_FUZZ_TARGET(NULL, s2n_fuzz_test, NULL)
