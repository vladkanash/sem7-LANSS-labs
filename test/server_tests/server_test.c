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

char input[] = "TIME\n";
char long_input[] = "ECHO A very long string to echo it :D\r\n";

server_command init_server_command() {
  server_command command;
  memset(&command, 0, sizeof(server_command));
  command.state = INITIAL;
  command.success = false;
  return command;
}

static void init_commands_test() {
  init_commands();

  int commands_count = sizeof(command_info)/sizeof(command_info[0]);

  assert_int_equal(4, commands_count);
}

static void parse_command_test() {
  server_command command = init_server_command();

  bool result = parse_command(input, &command);

  assert_true(result);
  assert_true(command.success);
  assert_string_equal(input, command.text);
}

static void get_long_command_test() {
  server_command command = init_server_command();

  bool result = get_long_command(long_input, &command);

  assert_true(result);
  assert_string_equal(long_input, command.text);
}

static void get_command_test() {
  server_command response = get_command(input);

  assert_string_equal(response.text, input);
  assert_true(response.success);
}

static void process_command_test() {
  server_command command = get_command(input);

  command_response result = process_command(command);

  assert_true(result.success);
}

static void find_line_ending_test() {
  int result = find_line_ending(input);

  assert_int_not_equal(result, 0);

  result = find_line_ending("");

  assert_int_equal(result, 0);
}

static void get_current_time_test() {
  server_command command = get_command(input);
  command_response result = process_command(command);

  get_current_time(&result);

  assert_int_equal(result.text_length, 30);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(init_commands_test),
    cmocka_unit_test(parse_command_test),
    cmocka_unit_test(get_long_command_test),
    cmocka_unit_test(get_command_test),
    cmocka_unit_test(process_command_test),
    cmocka_unit_test(find_line_ending_test),
    cmocka_unit_test(get_current_time_test),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
