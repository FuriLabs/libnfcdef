/*
 * Copyright (C) 2019-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2019 Jolla Ltd.
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

#include "ndef_util.h"
#include "ndef_rec_p.h"

#include <gutil_log.h>
#include <gutil_misc.h>

static TestOpt test_opt;
static const char* test_system_locale = NULL;

/* Stubs */

const char*
ndef_system_locale(
    void)
{
    return test_system_locale;
}

/* Utilities */

static
void
test_dump_data(
    const GUtilData* data)
{
    if (GLOG_ENABLED(GLOG_LEVEL_DEBUG)) {
        const guint8* ptr = data->bytes;
        gsize len = data->size;
        guint off = 0;

        while (len > 0) {
            char buf[GUTIL_HEXDUMP_BUFSIZE];
            const guint consumed = gutil_hexdump(buf, ptr + off, len);

            GDEBUG("    %04X: %s", off, buf);
            len -= consumed;
            off += consumed;
        }
    }
}
/*==========================================================================*
 * null
 *==========================================================================*/

static
void
test_null(
    void)
{
    NdefData ndef;

    memset(&ndef, 0, sizeof(ndef));
    g_assert(!ndef_rec_sp_new_from_data(NULL));
    g_assert(!ndef_rec_sp_new_from_data(&ndef));
    g_assert(!ndef_rec_sp_new(NULL, NULL, NULL, NULL, 0, 0, NULL));
}

/*==========================================================================*
 * valid
 *==========================================================================*/

/* Table 4. Example for a Simple URI */
static const guint8 test_valid_table4[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x12,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */

    0xd1,         /* NDEF record header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g'
};

/* Table 5. Example for a Complex URI */
static const guint8 test_valid_table5[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x49,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */

    0x81,         /* NDEF header (MB=1, ME=0, SR=0, TNF = 0x01) */
    0x01,         /* Record name length (1 byte) */
    0x00, 0x00,
    0x00, 0x0e,   /* The length of the URI payload (long format) */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x11,         /* NDEF header (SR=1, TNF=0x01) */
    0x03,         /* The length of the record name */
    0x01,         /* The length of the "act" payload */
    'a','c','t',  /* Record type: "act" */
    0x00,         /* Action = Launch browser */

    0x11,         /* NDEF header (SR=1, TNF=0x01) */
    0x01,         /* Length of the record name */
    0x12,         /* Length of the record payload */
    'T',          /* Record type: 'T' (Text) */
    0x05,         /* Status byte (UTF-8, five-byte code) */
    'e','n','-','U','S',
    'H','e','l','l','o',',',' ','w','o','r','l','d',

    0x51,         /* NDEF header (SR=1, ME=1, TNF= 0x01) */
    0x01,         /* Record name length */
    0x13,         /* Length of the Text payload */
    'T',          /* Record type: 'T' (Text) */
    0x02,         /* Status byte (UTF-8, two-byte language code) */
    'f','i',
    'M','o','r','j','e','n','s',',',' ','m','a','a','i','l','m','a'
};

static const guint8 test_valid_es[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x57,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */

    0x91,         /* NDEF header (MB=1, ME=0, SR=0, TNF = 0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x11,         /* NDEF header (SR=1, TNF=0x01) */
    0x03,         /* The length of the record name */
    0x01,         /* The length of the "act" payload */
    'a','c','t',  /* Record type: "act" */
    0x00,         /* Action = Launch browser */

    0x11,         /* NDEF header (SR=1, TNF=0x01) */
    0x01,         /* Length of the record name */
    0x12,         /* Length of the record payload */
    'T',          /* Record type: 'T' (Text) */
    0x05,         /* Status byte (UTF-8, five-byte code) */
    'e','n','-','U','S',
    'H','e','l','l','o',',',' ','w','o','r','l','d',

    0x11,         /* NDEF header (SR=1, TNF=0x01) */
    0x01,         /* Length of the record name */
    0x0d,         /* Length of the record payload */
    'T',          /* Record type: 'T' (Text) */
    0x02,         /* Status byte (UTF-8, 2-byte code) */
    'e','s',
    'H','o','l','a',' ','M','u','n','d','o',

    0x51,         /* NDEF header (SR=1, ME=1, TNF= 0x01) */
    0x01,         /* Record name length */
    0x13,         /* Length of the Text payload */
    'T',          /* Record type: 'T' (Text) */
    0x02,         /* Status byte (UTF-8, two-byte language code) */
    'f','i',
    'M','o','r','j','e','n','s',',',' ','m','a','a','i','l','m','a'
};

static const guint8 test_valid_x[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x17,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x01,         /* The length of the record name */
    0x00,         /* No payload */
    'x'           /* Record type: 'x' (ignored) */
};

static const guint8 test_valid_ignore_empty[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x16,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x50,         /* NDEF header (ME=1, SR=1, TNF=0x00) */
    0x00,         /* The length of the record name */
    0x00          /* Payload length */
};

static const guint8 test_valid_bad_icon_type1[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x19,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x52,         /* NDEF header (ME=1, SR=1, TNF=0x02) */
    0x03,         /* The length of the record name */
    0x00,         /* Payload length */
    'f','o','o'   /* Icon mime type "foo" (ignored) */
};

static const guint8 test_valid_bad_icon_type2[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x1a,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x52,         /* NDEF header (ME=1, SR=1, TNF=0x02) */
    0x01,         /* The length of the record name */
    0x03,         /* Payload length */
    ' ',          /* Mime record of type "foo" (invalid) */
    'b','a','r'
};

static const guint8 test_valid_bad_icon_type3[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x20,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x52,         /* NDEF header (ME=1, SR=1, TNF=0x02) */
    0x07,         /* The length of the record name */
    0x03,         /* Payload length */
    'f','o','o','/','b','a','r',
    'f','o','o'
};

static const guint8 test_valid_icon_image[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x22,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x52,         /* NDEF header (ME=1, SR=1, TNF=0x02) */
    0x09,         /* The length of the record name */
    0x03,         /* Payload length */
    'i','m','a','g','e','/','f','o','o',
    'f','o','o'
};

static const guint8 test_valid_icon_video[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x22,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */

    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x52,         /* NDEF header (ME=1, SR=1, TNF=0x02) */
    0x09,         /* The length of the record name */
    0x03,         /* Payload length */
    'v','i','d','e','o','/','f','o','o',
    'f','o','o'
};

static const guint8 test_valid_icon_image_video[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x31,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x12,         /* NDEF header (SR=1, TNF=0x02) */
    0x09,         /* The length of the record name */
    0x03,         /* Payload length */
    'i','m','a','g','e','/','f','o','o',
    'f','o','o',

    0x52,         /* NDEF header (ME=1, SR=1, TNF=0x02) */
    0x09,         /* The length of the record name */
    0x03,         /* Payload length */
    'v','i','d','e','o','/','f','o','o',
    'f','o','o'
};

static const guint8 test_valid_size[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x29,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x11,         /* NDEF header (SR=1, TNF=0x01) */
    0x01,         /* The length of the record name */
    0x03,         /* The length of the 's' payload (invalid) */
    's',          /* Record type: 's' */
    0x01, 0x02, 0x03, /* Ignored */

    0x11,         /* NDEF header (SR=1, TNF=0x01) */
    0x01,         /* The length of the record name */
    0x04,         /* The length of the 's' payload */
    's',          /* Record type: 's' */
    0x01, 0x02, 0x03, 0x04,

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x01,         /* The length of the record name */
    0x04,         /* The length of the 's' payload */
    's',          /* Record type: 's' */
    0x00, 0x01, 0x02, 0x03 /* Ignored */
};

static const guint8 test_valid_save[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x19,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x03,         /* The length of the record name */
    0x01,         /* The length of the "act" payload */
    'a','c','t',  /* Record type: "act" */
    0x01          /* Action = Save */
};

static const guint8 test_valid_edit[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x19,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x03,         /* The length of the record name */
    0x01,         /* The length of the "act" payload */
    'a','c','t',  /* Record type: "act" */
    0x02          /* Action = Edit */
};

static const guint8 test_valid_twoacts[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x20,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x11,         /* NDEF header (SR=1, TNF=0x01) */
    0x03,         /* The length of the record name */
    0x01,         /* The length of the "act" payload */
    'a','c','t',  /* Record type: "act" */
    0x01,         /* Action = Save */

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x03,         /* The length of the record name */
    0x01,         /* The length of the "act" payload */
    'a','c','t',  /* Record type: "act" */
    0x02          /* Action = Edit */
};

static const guint8 test_valid_badact1[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x19,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x03,         /* The length of the record name */
    0x01,         /* The length of the "act" payload */
    'a','c','t',  /* Record type: "act" */
    0x03          /* Action (invalid) */
};

static const guint8 test_valid_badact2[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x1a,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */

    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x03,         /* The length of the record name */
    0x02,         /* The length of the "act" payload (invalid) */
    'a','c','t',  /* Record type: "act" */
    0x00, 0x01    /* Action (invalid) */
};

static const guint8 test_valid_type[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x2d,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */
    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g',

    0x11,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x01,         /* The length of the record name */
    0x01,         /* The length of the 't' payload */
    't',          /* Record type: 't' */
    ' ',          /* Ignored (invalid) */

    0x11,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x01,         /* The length of the record name */
    0x07,         /* The length of the 't' payload */
    't',          /* Record type: 't' */
    'f','o','o','/','b','a','r',

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x01,         /* The length of the record name */
    0x07,         /* The length of the 't' payload */
    't',          /* Record type: 't' */
    'b','a','r','/','f','o','o' /* Ignored */
};

static const guint8 test_data_foo[] = { 'f', 'o', 'o' };

#define NO_ICON { NULL, 0 }, NULL

typedef struct test_valid_data {
    const char* name;
    const char* locale;
    GUtilData rec;
    const char* uri;
    const char* title;
    const char* lang;
    const char* type;
    guint size;
    NDEF_SP_ACT act;
    NdefMedia icon;
} TestValidData;

static const TestValidData valid_tests[] = {
    {
        "table4",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_table4) },
        "http://www.nfc-forum.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
    },{
        "table5",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_table5) },
        "http://www.nfc-forum.org",
        "Hello, world", "en-US",
        NULL, 0, NDEF_SP_ACT_OPEN, { NO_ICON }
    },{
        "table5/en",
        "en", { TEST_ARRAY_AND_SIZE(test_valid_table5) },
        "http://www.nfc-forum.org",
        "Hello, world", "en-US",
        NULL, 0, NDEF_SP_ACT_OPEN, { NO_ICON }
    },{
        "table5/fi",
        "fi", { TEST_ARRAY_AND_SIZE(test_valid_table5) },
        "http://www.nfc-forum.org",
        "Morjens, maailma", "fi",
        NULL, 0, NDEF_SP_ACT_OPEN, { NO_ICON }
    },{
        "table5/es",
        "es", { TEST_ARRAY_AND_SIZE(test_valid_es) },
        "http://www.nfc-forum.org",
        "Hola Mundo", "es",
        NULL, 0, NDEF_SP_ACT_OPEN, { NO_ICON }
    },{
        "x",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_x) },
        "https://www.sailfishos.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
    },{
        "ignore_empty",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_ignore_empty) },
        "https://www.sailfishos.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
    },{
        "bad_icon_type1",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_bad_icon_type1) },
        "https://www.sailfishos.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
    },{
        "bad_icon_type2",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_bad_icon_type2) },
        "https://www.sailfishos.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
    },{
        "bad_icon_type3",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_bad_icon_type3) },
        "https://www.sailfishos.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
    },{
        "icon_image",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_icon_image) },
        "https://www.sailfishos.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT,
        { { TEST_ARRAY_AND_SIZE(test_data_foo) }, "image/foo" }
    },{
        "icon_video",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_icon_video) },
        "https://www.sailfishos.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT,
        { { TEST_ARRAY_AND_SIZE(test_data_foo) }, "video/foo" }
    },{
        "icon_image_video",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_icon_image_video) },
        "https://www.sailfishos.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT,
        { { TEST_ARRAY_AND_SIZE(test_data_foo) }, "image/foo" }
    },{
        "size",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_size) },
        "http://www.nfc-forum.org",
        NULL, NULL, NULL, 0x01020304, NDEF_SP_ACT_DEFAULT, { NO_ICON }
    },{
        "save",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_save) },
        "http://www.nfc-forum.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_SAVE, { NO_ICON }
    },{
        "edit",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_edit) },
        "http://www.nfc-forum.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_EDIT, { NO_ICON }
    },{
        "twoacts",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_twoacts) },
        "http://www.nfc-forum.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_SAVE, { NO_ICON }
    },{
        "badact1",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_badact1) },
        "http://www.nfc-forum.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
    },{
        "badact2",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_badact2) },
        "http://www.nfc-forum.org",
        NULL, NULL, NULL, 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
     },{
        "type",
        NULL, { TEST_ARRAY_AND_SIZE(test_valid_type) },
        "http://www.nfc-forum.org",
        NULL, NULL, "foo/bar", 0, NDEF_SP_ACT_DEFAULT, { NO_ICON }
   }
};

static
void
test_valid_check(
    NdefRecSp* sp,
    const TestValidData* test)
{
    g_assert(sp);
    g_assert_cmpint(sp->rec.tnf, == ,NDEF_TNF_WELL_KNOWN);
    g_assert_cmpint(sp->rec.rtd, == ,NDEF_RTD_SMART_POSTER);
    g_assert_cmpstr(sp->uri, == ,test->uri);
    g_assert_cmpstr(sp->title, == ,test->title);
    g_assert_cmpstr(sp->lang, == ,test->lang);
    g_assert_cmpstr(sp->type, == ,test->type);
    g_assert_cmpuint(sp->size, == ,test->size);
    g_assert_cmpint(sp->act, == ,test->act);
    if (test->icon.data.bytes) {
        g_assert(sp->icon);
        g_assert_cmpstr(sp->icon->type, == ,test->icon.type);
    } else {
        g_assert(!sp->icon);
    }
}

static
void
test_valid(
    gconstpointer data)
{
    const TestValidData* test = data;
    NdefData ndef;
    NdefRecSp* sp;
    NdefRec* rec;

    memset(&ndef, 0, sizeof(ndef));
    ndef.rec = test->rec;
    ndef.payload_length = ndef.rec.bytes[2];
    ndef.type_offset = 3;
    ndef.type_length = ndef.rec.bytes[1];

    test_system_locale = test->locale;
    sp = ndef_rec_sp_new_from_data(&ndef);
    test_valid_check(sp, test);
    ndef_rec_unref(&sp->rec);

    rec = ndef_rec_new(&test->rec);
    g_assert(rec);
    g_assert(NDEF_IS_REC_SP(rec));
    test_valid_check(NDEF_REC_SP(rec), test);
    ndef_rec_unref(rec);
}

static
void
test_encode(
    gconstpointer data)
{
    const TestValidData* test = data;
    NdefRecSp* enc;
    NdefRec* dec;

    test_system_locale = test->locale;
    enc = ndef_rec_sp_new(test->uri, test->title, test->lang, test->type,
        test->size, test->act, test->icon.data.bytes ? &test->icon : NULL);
    g_assert(enc);
    GDEBUG("Encoded record:");
    test_dump_data(&enc->rec.raw);
    test_valid_check(enc, test);

    dec = ndef_rec_new(&enc->rec.raw);
    g_assert(dec);
    g_assert(NDEF_IS_REC_SP(dec));
    test_valid_check(NDEF_REC_SP(dec), test);

    ndef_rec_unref(&enc->rec);
    ndef_rec_unref(dec);
}

/*==========================================================================*
 * invalid
 *==========================================================================*/

static const guint8 test_invalid_uri0[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x04,         /* Data length */
    'S','p',      /* The record name "Sp" */

    0xd1,         /* NDEF record header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x01,         /* The length of the record name */
    0x00,         /* No payload */
    'x'           /* Record type: 'x' (ignored) */
};

static const guint8 test_invalid_uri2[] = {
    0xd1,         /* NDEF header (MB=1, ME=1, SR=1, TNF=0x01) */
    0x02,         /* Record name length */
    0x25,         /* Length of the Smart Poster data */
    'S','p',      /* The record name "Sp" */

    0x91,         /* NDEF record header (MB=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0f,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x02,         /* Abbreviation: "https://www." */
    's','a','i','l','f','i','s','h','o','s','.','o','r','g',

    0x51,         /* NDEF header (ME=1, SR=1, TNF=0x01) */
    0x01,         /* Record name length (1 byte) */
    0x0e,         /* The length of the URI payload */
   'U',           /* Record type: 'U' (URI) */
    0x01,         /* Abbreviation: "http://www." */
    'n','f','c','-','f','o','r','u','m','.','o','r','g'
};

typedef struct test_invalid_data {
    const char* name;
    GUtilData rec;
} TestInvalidData;

static const TestInvalidData invalid_tests[] = {
    { "uri0", { TEST_ARRAY_AND_SIZE(test_invalid_uri0) } },
    { "uri2", { TEST_ARRAY_AND_SIZE(test_invalid_uri2) } }
};

static
void
test_invalid(
    gconstpointer data)
{
    const TestInvalidData* test = data;
    NdefData ndef;
    NdefRec* rec;

    memset(&ndef, 0, sizeof(ndef));
    ndef.rec = test->rec;
    ndef.payload_length = ndef.rec.bytes[2];
    ndef.type_offset = 3;
    ndef.type_length = ndef.rec.bytes[1];

    g_assert(!ndef_rec_sp_new_from_data(&ndef));

    /* ndef_rec_new turns it into a generic record */
    rec = ndef_rec_new(&test->rec);
    g_assert(!NDEF_IS_REC_SP(rec));
    ndef_rec_unref(rec);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(t) "/ndef_rec_sp/" t

int main(int argc, char* argv[])
{
    guint i;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
    g_type_init();
    G_GNUC_END_IGNORE_DEPRECATIONS;
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("null"), test_null);
    for (i = 0; i < G_N_ELEMENTS(valid_tests); i++) {
        const TestValidData* test = valid_tests + i;
        char* path = g_strconcat(TEST_("/valid/"), test->name, NULL);

        g_test_add_data_func(path, test, test_valid);
        g_free(path);
    }
    for (i = 0; i < G_N_ELEMENTS(invalid_tests); i++) {
        const TestInvalidData* test = invalid_tests + i;
        char* path = g_strconcat(TEST_("/invalid/"), test->name, NULL);

        g_test_add_data_func(path, test, test_invalid);
        g_free(path);
    }
    for (i = 0; i < G_N_ELEMENTS(valid_tests); i++) {
        const TestValidData* test = valid_tests + i;
        char* path = g_strconcat(TEST_("/encode/"), test->name, NULL);

        g_test_add_data_func(path, test, test_encode);
        g_free(path);
    }
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
