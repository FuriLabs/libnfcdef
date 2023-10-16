/*
 * Copyright (C) 2018-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2019 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#ifndef NDEF_TLV_H
#define NDEF_TLV_H

#include "ndef_types.h"

G_BEGIN_DECLS

/*
 * Utilities for parsing NDEF Message TLV blocks in the format defined by
 * NFCForum-TS-Type-2-Tag specification.
 *
 * NULL TLVs are silently skipped, zero is returned when TLV_TERMINATOR
 * is encountered. Usage:
 *
 * guint type;
 * GUtilData buf;
 * GUtilData value;
 *
 * ... Initialize buf
 *
 * while ((type = ndef_tlv_next(&buf, &value)) > 0) {
 *   ... analize type and value
 * }
 */
#define TLV_NULL            (0)
#define TLV_LOCK_CONTROL    (1)
#define TLV_MEMORY_CONTROL  (2)
#define TLV_NDEF_MESSAGE    (3)
#define TLV_TERMINATOR      (254)

guint
ndef_tlv_next(
    GUtilData* buf,
    GUtilData* value);

/*
 * ndef_tlv_check() returns the actual size of TLV sequence including
 * TLV_TERMINATOR, zero if the sequence is incomplete or optentially
 * broken.
 */
gsize
ndef_tlv_check(
    const GUtilData* buf);

G_END_DECLS

#endif /* NDEF_TLV_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
