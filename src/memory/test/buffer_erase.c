/**
 * @file   buffer_erase.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2010
 * @date   Thu Jan 21 18:27:58 2010
 *
 * @brief  buffer_erase unit test
 *
 *
 */

#include	"rinoo/rinoo.h"

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int		main()
{
  t_buffer	*buf;

  buf = buffer_create(10, 42);
  XTEST(buf != NULL);
  XTEST(buf->buf != NULL);
  XTEST(buf->len == 0);
  XTEST(buf->size == 10);
  XTEST(buf->max_size == 42);
  XTEST(buffer_add(buf, "bl1bl1bl1", 9) == 9);
  XTEST(buf->buf != NULL);
  XTEST(memcmp(buf->buf, "bl1bl1bl1", 9) == 0);
  XTEST(buf->len == 9);
  XTEST(buf->size == 10);
  XTEST(buf->max_size == 42);
  XTEST(buffer_add(buf, "bl2bl2bl2", 9) == 9);
  XTEST(buf->buf != NULL);
  XTEST(memcmp(buf->buf, "bl1bl1bl1bl2bl2bl2", 18) == 0);
  XTEST(buf->len == 18);
  XTEST(buf->size == 42);
  XTEST(buf->max_size == 42);
  XTEST(buffer_erase(buf, 9) == 1);
  XTEST(buf->buf != NULL);
  XTEST(memcmp(buf->buf, "bl2bl2bl2", 9) == 0);
  XTEST(buf->len == 9);
  XTEST(buf->size == 42);
  XTEST(buf->max_size == 42);
  XTEST(buffer_erase(buf, 9) == 1);
  XTEST(buf->buf != NULL);
  XTEST(buf->len == 0);
  XTEST(buf->size == 42);
  XTEST(buf->max_size == 42);
  buffer_destroy(buf);
  XPASS();
}