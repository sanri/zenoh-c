/*
 * Copyright (c) 2017, 2020 ADLINK Technology Inc.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
 * which is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *
 * Contributors:
 *   ADLINK zenoh team, <zenoh@adlink-labs.tech>
 */
#include <stdio.h>
#include <string.h>
#include "zenoh.h"

int main(int argc, char **argv)
{
  z_init_logger();

  if (argc < 2)
  {
    printf("USAGE:\n\tzn_pub_thr <payload-size> [<zenoh-locator>]\n\n");
    exit(-1);
  }
  size_t len = atoi(argv[1]);
  printf("Running throughput test for payload of %zu bytes.\n", len);
  z_owned_config_t config = z_config_default();
  if (argc > 2)
  {
    z_config_set(z_borrow(config), ZN_CONFIG_PEER_KEY, z_string_new(argv[2]));
  }

  z_owned_session_t os = z_open(z_move(config));
  if (!z_check(os))
  {
    printf("Unable to open session!\n");
    exit(-1);
  }
  z_session_t s = z_borrow(os);

  char *data = (char *)malloc(len);
  memset(data, 1, len);

  z_owned_reskey_t oreskey = z_register_resource(s, z_rname("/test/thr"));
  z_reskey_t reskey = z_borrow(oreskey);
  z_owned_publisher_t pub = z_register_publisher(s, reskey);
  if (!z_check(pub))
  {
    printf("Unable to declare publisher.\n");
    exit(-1);
  }

  while (1)
  {
    z_write(s, reskey, (const uint8_t *)data, len);
  }
  z_unregister_publisher(&pub);
  z_reskey_free(z_move(oreskey));
  z_close(z_move(os));
}