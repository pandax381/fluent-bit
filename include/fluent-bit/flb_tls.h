/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2015-2016 Treasure Data Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef FLB_TLS_H
#define FLB_TLS_H

#ifdef HAVE_TLS

#include <fluent-bit/flb_thread.h>
#include <fluent-bit/flb_output.h>

int net_io_tls_write(struct flb_thread *th, struct flb_io_upstream *u,
                     void *data, size_t len, size_t *out_len);
int net_io_tls_read(struct flb_thread *th, struct flb_io_upstream *u,
                    void *buf, size_t len);

int flb_io_tls_connect(struct flb_io_upstream *u,
                       struct flb_thread *th);

#endif /* HAVE_TLS */
#endif
