#include "test_common.h"
/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-03    chenyong      the first version
 */

extern "C" {
#include "../pikascript-lib/requests/webclient.h"

#define GET_HEADER_BUFSZ 1024
#define GET_RESP_BUFSZ 1024

#define GET_LOCAL_URI "http://www.rt-thread.com/service/rt-thread.txt"

/* send HTTP GET request by common request interface, it used to receive
 * longer data */
static int webclient_get_comm(const char* uri) {
    struct webclient_session* session = RT_NULL;
    unsigned char* buffer = RT_NULL;
    int index = 0;
    int ret = 0;
    int bytes_read, resp_status;
    int content_length = -1;

    buffer = (unsigned char*)web_malloc(GET_RESP_BUFSZ);
    if (buffer == RT_NULL) {
        rt_kprintf("no memory for receive buffer.\n");
        ret = -RT_ENOMEM;
        goto __exit;
    }

    /* create webclient session and set header response size */
    session = webclient_session_create(GET_HEADER_BUFSZ);
    if (session == RT_NULL) {
        ret = -RT_ENOMEM;
        goto __exit;
    }

    /* send GET request by default header */
    if ((resp_status = webclient_get(session, uri)) != 200) {
        rt_kprintf("webclient GET request failed, response(%d) error.\n",
                   resp_status);
        ret = -RT_ERROR;
        goto __exit;
    }

    rt_kprintf("webclient get response data: \n");

    content_length = webclient_content_length_get(session);
    if (content_length < 0) {
        rt_kprintf("webclient GET request type is chunked.\n");
        do {
            bytes_read = webclient_read(session, (void*)buffer, GET_RESP_BUFSZ);
            if (bytes_read <= 0) {
                break;
            }

            for (index = 0; index < bytes_read; index++) {
                rt_kprintf("%c", buffer[index]);
            }
        } while (1);

        rt_kprintf("\n");
    } else {
        int content_pos = 0;

        do {
            bytes_read =
                webclient_read(session, (void*)buffer,
                               content_length - content_pos > GET_RESP_BUFSZ
                                   ? GET_RESP_BUFSZ
                                   : content_length - content_pos);
            if (bytes_read <= 0) {
                break;
            }

            for (index = 0; index < bytes_read; index++) {
                rt_kprintf("%c", buffer[index]);
            }

            content_pos += bytes_read;
        } while (content_pos < content_length);

        rt_kprintf("\n");
    }

__exit:
    if (session) {
        webclient_close(session);
    }

    if (buffer) {
        web_free(buffer);
    }

    return ret;
}

/* send HTTP GET request by simplify request interface, it used to received
 * shorter data */
static int webclient_get_smpl(const char* uri) {
    char* response = RT_NULL;
    size_t resp_len = 0;
    int index;

    if (webclient_request(uri, RT_NULL, RT_NULL, 0, (void**)&response,
                          &resp_len) < 0) {
        rt_kprintf("webclient send get request failed.");
        return -RT_ERROR;
    }

    rt_kprintf("webclient send get request by simplify request interface.\n");
    rt_kprintf("webclient get response data: \n");
    for (index = 0; index < (int)strlen(response); index++) {
        rt_kprintf("%c", response[index]);
    }
    rt_kprintf("\n");

    if (response) {
        web_free(response);
    }

    return 0;
}

int webclient_get_test(int argc, char** argv) {
    char* uri = RT_NULL;
    if (argc == 1) {
        uri = web_strdup(GET_LOCAL_URI);
        if (uri == RT_NULL) {
            rt_kprintf("no memory for create get request uri buffer.\n");
            return -RT_ENOMEM;
        }

        if (RT_EOK != webclient_get_comm(uri)) {
            return -RT_EIO;
        }
    } else if (argc == 2) {
        if (strcmp(argv[1], "-s") == 0) {
            uri = web_strdup(GET_LOCAL_URI);
            if (uri == RT_NULL) {
                rt_kprintf("no memory for create get request uri buffer.\n");
                return -RT_ENOMEM;
            }

            if (RT_EOK != webclient_get_smpl(uri)) {
                return -RT_EIO;
            }
        } else {
            uri = web_strdup(argv[1]);
            if (uri == RT_NULL) {
                rt_kprintf("no memory for create get request uri buffer.\n");
                return -RT_ENOMEM;
            }
            if (RT_EOK != webclient_get_comm(uri)) {
                return RT_EIO;
            }
        }
    } else if (argc == 3 && strcmp(argv[1], "-s") == 0) {
        uri = web_strdup(argv[2]);
        if (uri == RT_NULL) {
            rt_kprintf("no memory for create get request uri buffer.\n");
            return -RT_ENOMEM;
        }

        if (RT_EOK != webclient_get_smpl(uri)) {
            return -RT_EIO;
        }
    } else {
        rt_kprintf("web_get_test [URI]     - webclient GET request test.\n");
        rt_kprintf(
            "web_get_test -s [URI]  - webclient simplify GET request "
            "test.\n");
        return -RT_ERROR;
    }

    if (uri) {
        web_free(uri);
    }

    return RT_EOK;
}
}

#if !PIKA_NANO_ENABLE

// TEST(requests, webclient_get) {
//     char* argv1[] = {"test", "http://www.pikapython.com/index.html"};
//     EXPECT_EQ(webclient_get_test(2, argv1), 0);
//     char* argv2[] = {"test", "-s",
//                      "http://www.pikapython.com/index.html"};
//     EXPECT_EQ(webclient_get_test(3, argv2), 0);
// }

// TEST(requests, get) {
//     PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
//     extern unsigned char pikaModules_py_a[];
//     obj_linkLibrary(pikaMain, pikaModules_py_a);
//     obj_run(
//         pikaMain,
//         "import requests\n"
//         "r = requests.get('http://www.pikapython.com/index.html')\n");
//     /* assert */
//     // EXPECT_STREQ(
//     //     obj_getStr(pikaMain, "r.text"),
//     //     "RT-Thread is an open source IoT operating system from China,
//     which "
//     //     "has strong scalability: from a tiny kernel running on a tiny
//     core, "
//     //     "for example ARM Cortex-M0, or Cortex-M3/4/7, to a rich feature
//     system "
//     //     "running on MIPS32, ARM Cortex-A8, ARM Cortex-A9 DualCore
//     etc.\r\n");
//     /* deinit */
//     obj_deinit(pikaMain);
//     EXPECT_EQ(pikaMemNow(), 0);
// }

// TEST(requests, append_params_to_url) {
//     PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
//     extern unsigned char pikaModules_py_a[];
//     obj_linkLibrary(pikaMain, pikaModules_py_a);
//     pikaVM_runSingleFile(pikaMain,
//     "test/python/requests/requests_encode.py");
//     /* assert */
//     EXPECT_STREQ(log_buff[1], "'http://www.rt-thread.com?b=2&a=1'\r\n");
//     EXPECT_STREQ(log_buff[0], "'http://www.rt-thread.com?b=%25&a=+'\r\n");
//     /* deinit */
//     obj_deinit(pikaMain);
//     EXPECT_EQ(pikaMemNow(), 0);
// }

// TEST(requests, std_request) {
//     PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
//     extern unsigned char pikaModules_py_a[];
//     obj_linkLibrary(pikaMain, pikaModules_py_a);
//     pikaVM_runSingleFile(pikaMain,
//     "test/python/requests/requests_encode.py");
//     /* assert */
//     EXPECT_STREQ(log_buff[2], "HTTP/1.1 200 OK\r\n");
//     /* deinit */
//     obj_deinit(pikaMain);
//     EXPECT_EQ(pikaMemNow(), 0);
// }

// TEST(requests, get_gitee) {
//     PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
//     extern unsigned char pikaModules_py_a[];
//     obj_linkLibrary(pikaMain, pikaModules_py_a);
//     pikaVM_runSingleFile(pikaMain, "test/python/requests/gitee_issue.py");
//     /* assert */
//     EXPECT_STREQ(log_buff[1], "'http://www.rt-thread.com?b=2&a=1'\r\n");
//     EXPECT_STREQ(log_buff[0], "'http://www.rt-thread.com?b=%25&a=+'\r\n");
//     /* deinit */
//     obj_deinit(pikaMain);
//     EXPECT_EQ(pikaMemNow(), 0);
// }

TEST(requests, get_basic) {
    PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
    extern unsigned char pikaModules_py_a[];
    obj_linkLibrary(pikaMain, pikaModules_py_a);
    pikaVM_runSingleFile(pikaMain, "test/python/requests/get_basic.py");
    /* assert */
    EXPECT_STREQ(log_buff[2], "HTTP/1.1 200 OK\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}

//! Enable it manually if needed
#if 1
TEST(requests, post_data) {
    PikaObj* pikaMain = newRootObj("pikaMain", New_PikaMain);
    extern unsigned char pikaModules_py_a[];
    obj_linkLibrary(pikaMain, pikaModules_py_a);
    pikaVM_runSingleFile(pikaMain, "test/python/requests/post_data.py");
    /* assert */
    EXPECT_STREQ(log_buff[2], "HTTP/1.1 200 OK\r\n");
    /* deinit */
    obj_deinit(pikaMain);
    EXPECT_EQ(pikaMemNow(), 0);
}
#endif

#endif
