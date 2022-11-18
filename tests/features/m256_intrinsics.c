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
#include <immintrin.h>
#include <stdint.h>

#define ALIGNED_UINT8(N)            \
    union {                         \
        uint8_t coeffs[N];          \
        __m256i vec[(N + 31) / 32]; \
    }

int main()
{
    ALIGNED_UINT8(256) buf;
    return 0;
}
