//
// Created by vladkanash on 6.11.16.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

#include "../src/server/engine.h"
#include "../src/constants.h"

static command_holder command_info[COMMAND_COUNT];

static void init_commands_test() {
  init_commands();

  int commands_count = sizeof(command_info)/sizeof(command_info[0]);
  assert_int_equal(4, commands_count);
}

static void parse_command_test() {
  server_command command;
  memset(&command, 0, sizeof(server_command));
  command.state = INITIAL;
  command.success = false;

  char input[] = "TIME\n";

  bool result = parse_command(input, &command);

  assert_true(result);
}

static void get_command_test() {
  char buf[] = "TIME\n";

  server_command response = get_command(buf);

  assert_string_equal(response.text, buf);
}

static void process_command_test() {
  char buf[] = "TIME\n";
  server_command command = get_command(buf);

  command_response result = process_command(command);

  assert_true(result.success);
}

static void find_line_ending_test() {
  char buf[] = "TIME\n";
  int result = find_line_ending(buf);

  assert_int_not_equal(result, 0);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(init_commands_test),
    cmocka_unit_test(parse_command_test),
    cmocka_unit_test(get_command_test),
    cmocka_unit_test(process_command_test),
    cmocka_unit_test(find_line_ending_test),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
