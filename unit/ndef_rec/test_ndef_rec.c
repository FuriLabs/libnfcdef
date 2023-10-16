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

#include "test_common.h"

#include "ndef_rec_p.h"
#include "ndef_tlv.h"
#include "ndef_util_p.h"

#include <gutil_log.h>
#include <gutil_misc.h>

static TestOpt test_opt;

#define TLV_TEST (0x04)

/*==========================================================================*
 * locale
 *==========================================================================*/

static
void
test_locale(
    void)
{
    const char* l = ndef_system_locale();

    GDEBUG("locale = %s", l);
}

/*==========================================================================*
 * type
 *==========================================================================*/

static
void
test_type(
    void)
{
    static const guint8 rec[] = {
        0xd1,           /* NDEF record header (MB,ME,SR,TNF=0x01) */
        0x01,           /* Length of the record type */
        0x01,           /* Length of the record payload */
        'U',            /* Record type: 'U' (URI) */
        0x00
    };
    NdefData ndef;
    GUtilData type;

    memset(&ndef, 0, sizeof(ndef));
    memset(&type, 0, sizeof(type));
    g_assert(!ndef_type(NULL, &type));
    g_assert(!ndef_type(&ndef, &type));

    TEST_BYTES_SET(ndef.rec, rec);
    ndef.payload_length = rec[2];
    ndef.type_offset = 3;
    ndef.type_length = 1;
    g_assert(ndef_type(&ndef, &type));
    g_assert(type.bytes == rec + ndef.type_offset);
    g_assert_cmpuint(type.size, == ,ndef.type_length);
}

/*==========================================================================*
 * payload
 *==========================================================================*/

static
void
test_payload(
    void)
{
    static const guint8 rec[] = {
        0xd1,           /* NDEF record header (MB,ME,SR,TNF=0x01) */
        0x01,           /* Length of the record type */
        0x01,           /* Length of the record payload */
        'U',            /* Record type: 'U' (URI) */
        0x00
    };
    NdefData ndef;
    GUtilData payload;

    memset(&ndef, 0, sizeof(ndef));
    memset(&payload, 0, sizeof(payload));
    g_assert(!ndef_payload(NULL, &payload));
    g_assert(!ndef_payload(&ndef, &payload));

    TEST_BYTES_SET(ndef.rec, rec);
    ndef.payload_length = rec[2];
    ndef.type_offset = 3;
    ndef.type_length = 1;
    g_assert(ndef_payload(&ndef, &payload));
    g_assert(payload.bytes == rec + ndef.type_offset + ndef.type_length);
    g_assert_cmpuint(payload.size, == ,ndef.payload_length);
}

/*==========================================================================*
 * null
 *==========================================================================*/

static
void
test_null(
    void)
{
    /* NULL tolerance */
    g_assert(!ndef_rec_new(NULL));
    g_assert(!ndef_rec_new_from_tlv(NULL));
    g_assert(!ndef_rec_ref(NULL));
    g_assert(!ndef_rec_initialize(NULL, NDEF_RTD_UNKNOWN, NULL));
    ndef_rec_unref(NULL);
}

/*==========================================================================*
 * empty
 *==========================================================================*/

static
void
test_empty(
    void)
{
    GUtilData bytes;
    NdefRec* rec;

    /* Special case - empty NDEF */
    memset(&bytes, 0, sizeof(bytes));
    rec = ndef_rec_new(&bytes);

    g_assert(rec);
    g_assert(!rec->next);
    g_assert(ndef_rec_initialize(rec, NDEF_RTD_UNKNOWN, NULL) == rec);
    g_assert_cmpint(rec->tnf, == ,NDEF_TNF_EMPTY);
    g_assert_cmpint(rec->rtd, == ,NDEF_RTD_UNKNOWN);
    g_assert(ndef_rec_ref(rec) == rec);

    ndef_rec_unref(rec);
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * short
 *==========================================================================*/

static
void
test_short(
    void)
{
    static const guint8 data[] = { 0x01, 0x02 /* Arbitrary garbage */ };
    GUtilData bytes;

    TEST_BYTES_SET(bytes, data);
    g_assert(!ndef_rec_new(&bytes));
}

/*==========================================================================*
 * chunked
 *==========================================================================*/

static
void
test_chunked(
    void)
{
    /* Chunked records are not supported (yet?) */
    static const guint8 data[] = {
        0xf1,   /* NDEF record header (MB,ME,CF,SR,TNF=0x01) */
        0x01,   /* Length of the record type */
        0x00,   /* Length of the record payload */
        'U'
    };
    GUtilData bytes;

    TEST_BYTES_SET(bytes, data);
    g_assert(!ndef_rec_new(&bytes));
}

/*==========================================================================*
 * tlv
 *==========================================================================*/

static
void
test_tlv(
    void)
{
    static const guint8 tlv[] = {
        TLV_NULL,         /* NULL record */
        TLV_NDEF_MESSAGE, /* Value type */
        0x04,             /* Value length */
        0x91,                 /* NDEF record header (MB,SR,TNF=0x01) */
        0x01,                 /* Length of the record type */
        0x00,                 /* Length of the record payload */
        'x',                  /* Record type: 'x' */
        TLV_TERMINATOR    /* Terminator record */
    };
    GUtilData data, ndef;
    NdefRec* rec;

    ndef.bytes = tlv + 3;
    ndef.size = sizeof(tlv) - 4;
    TEST_BYTES_SET(data, tlv);
    rec = ndef_rec_new_from_tlv(&data);

    g_assert(rec);
    g_assert(!rec->next);
    g_assert(gutil_data_equal(&rec->raw, &ndef));
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * tlv_empty
 *==========================================================================*/

static
void
test_tlv_empty(
    void)
{
    static const guint8 tlv[] = {
        TLV_TEST,         /* Custom type (ignored) */
        0x00,             /* Value length */
        TLV_NDEF_MESSAGE, /* Value type */
        0x00,             /* Value length */
        TLV_TERMINATOR    /* Terminator record */
    };
    GUtilData data;
    NdefRec* rec;

    TEST_BYTES_SET(data, tlv);
    rec = ndef_rec_new_from_tlv(&data);

    g_assert(rec);
    g_assert(!rec->next);
    g_assert(ndef_rec_initialize(rec, NDEF_RTD_UNKNOWN, NULL) == rec);
    g_assert_cmpint(rec->tnf, == ,NDEF_TNF_EMPTY);
    g_assert_cmpint(rec->rtd, == ,NDEF_RTD_UNKNOWN);
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * tlv_complex
 *==========================================================================*/

static
void
test_tlv_complex(
    void)
{
    static const guint8 tlv[] = {
        TLV_NDEF_MESSAGE, 0x4f,
        0x91,       /* NDEF record header (MB,SR,TNF=0x01) */
        0x02,       /* Length of the record type */
        0x0a,       /* Length of the record payload */
        0x48, 0x73, /* Record type: "Hs" () */
        /* Payload */
        0x12, 0xd1, 0x02, 0x04, 0x61, 0x63, 0x01, 0x01, 0x30, 0x00,
        0x5a,       /* NDEF record header (ME,SR,IL,TNF=0x02) */
        0x20,       /* Length of the record type */
        0x1b,       /* Length of the record payload */
        0x01,       /* ID length */
        /* Record type: "application/vnd.bluetooth.ep.oob" */
        0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74,
        0x69, 0x6f, 0x6e, 0x2f, 0x76, 0x6e, 0x64, 0x2e,
        0x62, 0x6c, 0x75, 0x65, 0x74, 0x6f, 0x6f, 0x74,
        0x68, 0x2e, 0x65, 0x70, 0x2e, 0x6f, 0x6f, 0x62,
        0x30,       /* ID: "0" */
        /* Payload */
        0x1b, 0x00, 0x3b, 0x5a, 0xc0, 0xde, 0x1e, 0x00,
        0x0d, 0x09, 0x4e, 0x6f, 0x6b, 0x69, 0x61, 0x20,
        0x42, 0x48, 0x2d, 0x32, 0x31, 0x39, 0x04, 0x0d,
        0x04, 0x04, 0x20,
        TLV_TERMINATOR
    };

    GUtilData data;
    NdefRec* rec;
    NdefRec* rec2;

    TEST_BYTES_SET(data, tlv);
    rec = ndef_rec_new_from_tlv(&data);

    g_assert(rec);
    rec2 = rec->next;
    g_assert(rec2);
    g_assert(!rec2->next);

    /* First record */
    g_assert(rec->flags & NDEF_REC_FLAG_FIRST);
    g_assert(!(rec->flags & NDEF_REC_FLAG_LAST));
    g_assert_cmpuint(rec->raw.size, == ,(3 + tlv[3] + tlv[4]));
    g_assert(rec->raw.bytes);
    g_assert(!memcmp(rec->raw.bytes, tlv + 2, rec->raw.size));
    g_assert_cmpuint(rec->type.size, == ,rec->raw.bytes[1]);
    g_assert(rec->type.bytes == rec->raw.bytes + 3);
    g_assert_cmpuint(rec->id.size, == ,0);
    g_assert(!rec->id.bytes);
    g_assert_cmpuint(rec->payload.size, == ,tlv[4]);
    g_assert(rec->payload.bytes);
    g_assert(!memcmp(rec->payload.bytes, tlv + 7, rec->payload.size));

    /* Second record */
    g_assert(!(rec2->flags & NDEF_REC_FLAG_FIRST));
    g_assert(rec2->flags & NDEF_REC_FLAG_LAST);
    g_assert_cmpuint(rec2->raw.size, == ,(4 + tlv[18] + tlv[19] + tlv[20]));
    g_assert(rec2->raw.bytes);
    g_assert(!memcmp(rec2->raw.bytes, tlv + 17, rec->raw.size));
    g_assert_cmpuint(rec2->type.size, == ,rec2->raw.bytes[1]);
    g_assert(rec2->type.bytes == rec2->raw.bytes + 4);

    ndef_rec_unref(rec);
}

/*==========================================================================*
 * tlv_multiple
 *==========================================================================*/

static
void
test_tlv_multiple(
    void)
{
    static const guint8 tlv[] = {
        TLV_NULL,         /* NULL record */
        TLV_NDEF_MESSAGE, /* Value type */
        0x04,             /* Value length */
        0xd1,                 /* NDEF record header (MB,ME,SR,TNF=0x01) */
        0x01,                 /* Length of the record type */
        0x00,                 /* Length of the record payload */
        'x',                  /* Record type: 'x' */
        TLV_NDEF_MESSAGE, /* Value type */
        0x04,             /* Value length */
        /* This one is ignore because it's chunked */
        0xf1,                 /* NDEF record header (MB,ME,CF,SR,TNF=0x01) */
        0x01,                 /* Length of the record type */
        0x00,                 /* Length of the record payload */
        '-',                  /* Record type: '-' */
        TLV_NDEF_MESSAGE, /* Value type */
        0x06,             /* Value length */
        /* This one just broken, ignored too */
        0xc1,                   /* NDEF record header (MB,ME,TNF=0x01) */
        0x01,                   /* Length of the record type */
        0x00, 0x00, 0x00, 0xaa, /* Payload length (way beyond the end) */
        TLV_NDEF_MESSAGE, /* Value type */
        0x04,             /* Value length */
        0xd1,                 /* NDEF record header (MB,ME,SR,TNF=0x01) */
        0x01,                 /* Length of the record type */
        0x00,                 /* Length of the record payload */
        'y',                  /* Record type: 'y' */
        TLV_TERMINATOR    /* Terminator record */
    };
    GUtilData data;
    NdefRec* rec;

    TEST_BYTES_SET(data, tlv);
    rec = ndef_rec_new_from_tlv(&data);

    g_assert(rec);
    g_assert(rec->next);
    g_assert(!rec->next->next);

    ndef_rec_unref(rec);
}

/*==========================================================================*
 * no_type
 *==========================================================================*/

static
void
test_no_type(
    void)
{
    static const guint8 data[] = { 0xd0, 0x00, 0x00 };
    NdefRec* rec;
    GUtilData bytes;

    TEST_BYTES_SET(bytes, data);
    rec = ndef_rec_new(&bytes);
    g_assert(rec);
    g_assert_cmpint(rec->tnf, == ,NDEF_TNF_EMPTY);
    g_assert(!rec->type.size);
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * uri
 *==========================================================================*/

static
void
test_uri(
    void)
{
    static const guint8 data[] = {
        0xd1,           /* NDEF record header (MB,ME,SR,TNF=0x01) */
        0x01,           /* Length of the record type */
        0x0a,           /* Length of the record payload */
        'U',            /* Record type: 'U' (URI) */
        0x02,           /* "https://www." */
        'j', 'o', 'l', 'l', 'a', '.', 'c', 'o', 'm',
    };
    GUtilData bytes;
    NdefRec* rec;

    TEST_BYTES_SET(bytes, data);
    rec = ndef_rec_new(&bytes);

    g_assert(rec);
    g_assert(!rec->next);
    g_assert(NDEF_IS_REC_U(rec));
    g_assert(!g_strcmp0(NDEF_REC_U(rec)->uri, "https://www.jolla.com"));
    g_assert_cmpuint(rec->raw.size, == ,sizeof(data));
    g_assert(rec->raw.bytes);
    g_assert_cmpuint(rec->type.size, == ,rec->raw.bytes[1]);
    g_assert(rec->type.bytes == rec->raw.bytes + 3);
    g_assert_cmpuint(rec->payload.size, == ,rec->raw.bytes[2]);
    g_assert(rec->payload.bytes == rec->raw.bytes + 4);
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * well_known_short
 *==========================================================================*/

static
void
test_well_known_short(
    void)
{
    static const guint8 payload_bytes[] = {
        0x02,           /* "https://www." */
        'j', 'o', 'l', 'l', 'a', '.', 'c', 'o', 'm'
    };
    GUtilData payload;
    NdefRec* rec;
    NdefRec* urec;

    TEST_BYTES_SET(payload, payload_bytes);
    g_assert(!ndef_rec_new_well_known((GType)0, NDEF_RTD_URI,
        &ndef_rec_type_u, &payload));
    rec = ndef_rec_new_well_known(NDEF_TYPE_REC, NDEF_RTD_URI,
        &ndef_rec_type_u, &payload);
    g_assert(rec);

    /* Re-parse it */
    urec = ndef_rec_new(&rec->raw);
    g_assert(NDEF_IS_REC_U(urec));
    g_assert_cmpstr(NDEF_REC_U(urec)->uri, ==, "https://www.jolla.com");
    ndef_rec_unref(rec);
    ndef_rec_unref(urec);
}

/*==========================================================================*
 * well_known_long
 *==========================================================================*/

static
void
test_well_known_long(
    void)
{
    static const guint8 payload_bytes[] = {
        0x01,           /* "http://www." */
        'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c',
        'o', 'm', '/', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
        'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a'
    };
    GUtilData payload;
    NdefRec* rec;
    NdefRec* urec;

    TEST_BYTES_SET(payload, payload_bytes);
    rec = ndef_rec_new_well_known(NDEF_TYPE_REC, NDEF_RTD_URI,
        &ndef_rec_type_u, &payload);
    g_assert(rec);

    /* Re-parse it */
    urec = ndef_rec_new(&rec->raw);
    g_assert(NDEF_IS_REC_U(urec));
    g_assert_cmpstr(NDEF_REC_U(urec)->uri, == ,"http://www.example.com/"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaa");
    ndef_rec_unref(rec);
    ndef_rec_unref(urec);
}

/*==========================================================================*
 * broken_uri
 *==========================================================================*/

static
void
test_broken_uri(
    void)
{
    static const guint8 data[] = {
        0xd1,           /* NDEF record header (MB,ME,SR,TNF=0x01) */
        0x01,           /* Length of the record type */
        0x02,           /* Length of the record payload */
        'U',            /* Record type: 'U' (URI) */
        0x24,           /* The last valid prefix is 0x23 */
        0x00
    };
    GUtilData bytes;
    NdefRec* rec;

    TEST_BYTES_SET(bytes, data);
    rec = ndef_rec_new(&bytes);

    g_assert(rec);
    g_assert(!rec->next);
    g_assert(!NDEF_IS_REC_U(rec)); /* We treat it as a generic one */
    g_assert_cmpuint(rec->raw.size, == ,sizeof(data));
    g_assert(rec->raw.bytes);
    g_assert_cmpuint(rec->type.size, == ,rec->raw.bytes[1]);
    g_assert(rec->type.bytes == rec->raw.bytes + 3);
    g_assert_cmpuint(rec->payload.size, == ,rec->raw.bytes[2]);
    g_assert(rec->payload.bytes == rec->raw.bytes + 4);
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * mediatype
 *==========================================================================*/

static
void
test_mediatype(
    void)
{
    NdefRec* rec;
    GUtilData type, data;
    static const guint8 png[] = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a,
        0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0b,
        0x01, 0x03, 0x00, 0x00, 0x00, 0x48, 0xd9, 0x4f,
        0x47, 0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54,
        0x45, 0xff, 0xff, 0xff, 0x00, 0x2f, 0x6c, 0x03,
        0xda, 0xc6, 0x60, 0x00, 0x00, 0x00, 0x15, 0x49,
        0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0x60, 0x67,
        0x60, 0x60, 0x40, 0xc6, 0xff, 0xff, 0x1f, 0x80,
        0x63, 0x34, 0x39, 0x00, 0xba, 0xed, 0x08, 0x73,
        0xdb, 0x0d, 0xbb, 0xd3, 0x00, 0x00, 0x00, 0x00,
        0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
    };
    static const guint8 ndef_no_data[] = {
        0xd2, 0x18, 0x00, 0x61, 0x70, 0x70, 0x6c, 0x69,
        0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2f, 0x6f,
        0x63, 0x74, 0x65, 0x74, 0x2d, 0x73, 0x74, 0x72,
        0x65, 0x61, 0x6d
    };
    static const guint8 ndef_png[] = {
        0xd2, 0x09, 0x60, 0x69, 0x6d, 0x61, 0x67, 0x65,
        0x2f, 0x70, 0x6e, 0x67, 0x89, 0x50, 0x4e, 0x47,
        0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
        0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x12,
        0x00, 0x00, 0x00, 0x0b, 0x01, 0x03, 0x00, 0x00,
        0x00, 0x48, 0xd9, 0x4f, 0x47, 0x00, 0x00, 0x00,
        0x06, 0x50, 0x4c, 0x54, 0x45, 0xff, 0xff, 0xff,
        0x00, 0x2f, 0x6c, 0x03, 0xda, 0xc6, 0x60, 0x00,
        0x00, 0x00, 0x15, 0x49, 0x44, 0x41, 0x54, 0x08,
        0xd7, 0x63, 0x60, 0x67, 0x60, 0x60, 0x40, 0xc6,
        0xff, 0xff, 0x1f, 0x80, 0x63, 0x34, 0x39, 0x00,
        0xba, 0xed, 0x08, 0x73, 0xdb, 0x0d, 0xbb, 0xd3,
        0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
        0xae, 0x42, 0x60, 0x82
    };

    gutil_data_from_string(&type, "reallyreallyreallyreallyreallyreally"
        "reallyreallyreallyreallyreallyreallyreallyreallyreallyreally"
        "reallyreallyreallyreallyreallyreallyreallyreallyreallyreally"
        "reallyreallyreallyreallyreallyreallyreallyreallyreallyreally"
        "reallyreallyreallyreallyreallylong/mediatype");
    g_assert(!ndef_rec_new_mediatype(NULL, NULL));
    g_assert(!ndef_rec_new_mediatype(&type, NULL));

    gutil_data_from_string(&type, "application/octet-stream");
    rec = ndef_rec_new_mediatype(&type, NULL);
    g_assert(rec);
    g_assert_cmpuint(rec->raw.size, == ,sizeof(ndef_no_data));
    g_assert(!memcmp(rec->raw.bytes, ndef_no_data, rec->raw.size));
    ndef_rec_unref(rec);

    data.bytes = png;
    data.size = sizeof(png);
    gutil_data_from_string(&type, "image/png");
    rec = ndef_rec_new_mediatype(&type, &data);
    g_assert(rec);
    g_assert_cmpuint(rec->raw.size, == ,sizeof(ndef_png));
    g_assert(!memcmp(rec->raw.bytes, ndef_png, rec->raw.size));
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * valid_mediatype
 *==========================================================================*/

static
void
test_valid_mediatype(
    void)
{
    g_assert(!ndef_valid_mediatype_str("foo/bar", TRUE));
    g_assert(ndef_valid_mediatype_str("foo/bar", FALSE));
    g_assert(!ndef_valid_mediatype_str("foo/b", TRUE));
    g_assert(ndef_valid_mediatype_str("foo/b", FALSE));
    g_assert(ndef_valid_mediatype_str("foo/*", TRUE));
    g_assert(!ndef_valid_mediatype_str("foo/*", FALSE));
    g_assert(ndef_valid_mediatype_str("*/*", TRUE));
    g_assert(!ndef_valid_mediatype_str("*/*", FALSE));

    /* Various sorts of garbage */
    g_assert(!ndef_valid_mediatype(NULL, FALSE));
    g_assert(!ndef_valid_mediatype_str(NULL, FALSE));
    g_assert(!ndef_valid_mediatype_str("", TRUE));
    g_assert(!ndef_valid_mediatype_str("", FALSE));
    g_assert(!ndef_valid_mediatype_str("\x80", TRUE));
    g_assert(!ndef_valid_mediatype_str("\x80", FALSE));
    g_assert(!ndef_valid_mediatype_str("*/bar", TRUE));
    g_assert(!ndef_valid_mediatype_str("*/bar", FALSE));
    g_assert(!ndef_valid_mediatype_str("/", TRUE));
    g_assert(!ndef_valid_mediatype_str("/", FALSE));
    g_assert(!ndef_valid_mediatype_str("*", TRUE));
    g_assert(!ndef_valid_mediatype_str("*", FALSE));
    g_assert(!ndef_valid_mediatype_str("foo", TRUE));
    g_assert(!ndef_valid_mediatype_str("foo", FALSE));
    g_assert(!ndef_valid_mediatype_str("foo*", TRUE));
    g_assert(!ndef_valid_mediatype_str("foo*", FALSE));
    g_assert(!ndef_valid_mediatype_str("foo:bar", TRUE));
    g_assert(!ndef_valid_mediatype_str("foo:bar", FALSE));
    g_assert(!ndef_valid_mediatype_str("foo/", TRUE));
    g_assert(!ndef_valid_mediatype_str("foo/", FALSE));
    g_assert(!ndef_valid_mediatype_str("foo/bar/", TRUE));
    g_assert(!ndef_valid_mediatype_str("foo/bar/", FALSE));
}

/*==========================================================================*
 * id
 *==========================================================================*/

static
void
test_id(
    void)
{
    static const guint8 data[] = {
        0xd9,       /* NDEF record header (MB,ME,SR,IL,TNF=0x01) */
        0x01,       /* Length of the record type */
        0x00,       /* Length of the record payload */
        0x02,       /* ID length (2 bytes) */
        'x',        /* Record type: 'x' */
        'i', 'd',   /* Record id: 'id' */
    };
    GUtilData bytes;
    NdefRec* rec;

    TEST_BYTES_SET(bytes, data);
    rec = ndef_rec_new(&bytes);

    g_assert(rec);
    g_assert(!rec->next);
    g_assert(rec->flags & NDEF_REC_FLAG_FIRST);
    g_assert(rec->flags & NDEF_REC_FLAG_LAST);
    g_assert_cmpuint(rec->raw.size, == ,sizeof(data));
    g_assert(rec->raw.bytes);
    g_assert_cmpuint(rec->type.size, == ,rec->raw.bytes[1]);
    g_assert(rec->type.bytes == rec->raw.bytes + 4);
    g_assert_cmpuint(rec->id.size, == ,rec->raw.bytes[3]);
    g_assert(rec->id.bytes == rec->raw.bytes + 5);
    g_assert_cmpuint(rec->payload.size, == ,0);
    g_assert(!rec->payload.bytes);
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * unknown
 *==========================================================================*/

static
void
test_unknown(
    void)
{
    static const guint8 data[] = {
        0x91,       /* NDEF record header (MB,SR,TNF=0x01) */
        0x01,       /* Length of the record type */
        0x00,       /* Length of the record payload */
        'x',        /* Record type: 'x' */
    };
    GUtilData bytes;
    NdefRec* rec;

    TEST_BYTES_SET(bytes, data);
    rec = ndef_rec_new(&bytes);

    g_assert(rec);
    g_assert(!rec->next);
    g_assert_cmpuint(rec->raw.size, == ,sizeof(data));
    g_assert(rec->raw.bytes);
    g_assert(rec->flags & NDEF_REC_FLAG_FIRST);
    g_assert(!(rec->flags & NDEF_REC_FLAG_LAST));
    g_assert_cmpuint(rec->type.size, == ,rec->raw.bytes[1]);
    g_assert(rec->type.bytes == rec->raw.bytes + 3);
    g_assert_cmpuint(rec->payload.size, == ,0);
    g_assert(!rec->payload.bytes);
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * invalid_tnf
 *==========================================================================*/

static
void
test_invalid_tnf(
    void)
{
    static const guint8 data[] = {
        0x17,       /* NDEF record header (SR,TNF=0x07) */
        0x01,       /* Length of the record type */
        0x00,       /* Length of the record payload */
        'x',        /* Record type: 'x' */
    };
    GUtilData bytes;
    NdefRec* rec;

    TEST_BYTES_SET(bytes, data);
    rec = ndef_rec_new(&bytes);

    g_assert(rec);
    g_assert(!rec->next);
    g_assert(rec->tnf == NDEF_TNF_EMPTY); /* Default */
    g_assert(!(rec->flags & NDEF_REC_FLAG_FIRST));
    g_assert(!(rec->flags & NDEF_REC_FLAG_LAST));
    g_assert_cmpuint(rec->raw.size, == ,sizeof(data));
    g_assert(rec->raw.bytes);
    g_assert_cmpuint(rec->type.size, == ,rec->raw.bytes[1]);
    g_assert(rec->type.bytes == rec->raw.bytes + 3);
    g_assert_cmpuint(rec->payload.size, == ,0);
    g_assert(!rec->payload.bytes);
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * broken1
 *==========================================================================*/

static
void
test_broken1(
    void)
{
    static const guint8 data[] = {
        0xc1,                   /* NDEF record header (MB,ME,TNF=0x01) */
        0x01,                   /* Length of the record type */
        0xff, 0xee, 0xdd, 0xaa, /* Payload length (way beyond the end) */
        'x'                     /* Record type: 'x' */
    };
    GUtilData bytes;

    TEST_BYTES_SET(bytes, data);
    g_assert(!ndef_rec_new(&bytes));
}

/*==========================================================================*
 * broken2
 *==========================================================================*/

static
void
test_broken2(
    void)
{
    static const guint8 data[] = {
        0xc1,                   /* NDEF record header (MB,ME,TNF=0x01) */
        0x01,                   /* Length of the record type */
        0x00, 0x00, 0x00, 0xaa, /* Payload length (way beyond the end) */
        'x'                     /* Record type: 'x' */
    };
    GUtilData bytes;

    TEST_BYTES_SET(bytes, data);
    g_assert(!ndef_rec_new(&bytes));
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(t) "/ndef_rec/" t

int main(int argc, char* argv[])
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("locale"), test_locale);
    g_test_add_func(TEST_("type"), test_type);
    g_test_add_func(TEST_("payload"), test_payload);
    g_test_add_func(TEST_("null"), test_null);
    g_test_add_func(TEST_("empty"), test_empty);
    g_test_add_func(TEST_("short"), test_short);
    g_test_add_func(TEST_("chunked"), test_chunked);
    g_test_add_func(TEST_("tlv"), test_tlv);
    g_test_add_func(TEST_("tlv_empty"), test_tlv_empty);
    g_test_add_func(TEST_("tlv_complex"), test_tlv_complex);
    g_test_add_func(TEST_("tlv_multiple"), test_tlv_multiple);
    g_test_add_func(TEST_("no_type"), test_no_type);
    g_test_add_func(TEST_("uri"), test_uri);
    g_test_add_func(TEST_("well_known_short"), test_well_known_short);
    g_test_add_func(TEST_("well_known_long"), test_well_known_long);
    g_test_add_func(TEST_("mediatype"), test_mediatype);
    g_test_add_func(TEST_("valid_mediatype"), test_valid_mediatype);
    g_test_add_func(TEST_("broken_uri"), test_broken_uri);
    g_test_add_func(TEST_("id"), test_id);
    g_test_add_func(TEST_("unknown"), test_unknown);
    g_test_add_func(TEST_("invalid_tnf"), test_invalid_tnf);
    g_test_add_func(TEST_("broken1"), test_broken1);
    g_test_add_func(TEST_("broken2"), test_broken2);
    test_init(&test_opt, argc, argv);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
