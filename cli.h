#pragma once
#include <argp.h>
#include <string>

const char *argp_program_version =
  "rumi 0.1.0";
const char *argp_program_bug_address =
  "<mcshemail@gmail.com>";

/* A description of the arguments we accept. */
static char args_doc[] = "FILE.rum";

/* Program documentation. */
static char doc[] =
  "Rumi - a WIP compiler.";

/* The options we understand. */
static struct argp_option options[] = {
   {0,0,0,0, "Output options:" },
  {"output",   'o', "FILE",  0, "Output to FILE instead of standard output, ineffective for rumi" },
  {"llvm-ir", 'l', 0, 0, "emit llvm-ir code"},
  { 0 }
};

struct arguments
{
  char *arg1;                   /* arg1 */
  std::string *output_file;            /* file arg to ‘--output’ */
  char *input_file;
  int llvm_ir;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = (struct arguments*)state->input;

  switch (key)
    {
    case 'o':
      arguments->output_file = new std::string(arg);
      break;

    case 'l':
      arguments->llvm_ir = 1;
      break;

    case ARGP_KEY_NO_ARGS:
      argp_usage (state);

    case ARGP_KEY_ARG:
      arguments->input_file = arg;

      /* Now we consume all the rest of the arguments.
         state->next is the index in state->argv of the
         next argument to be parsed, which is the first string
         we’re interested in, so we can just use
         &state->argv[state->next] as the value for
         arguments->strings.

         In addition, by setting state->next to the end
         of the arguments, we can force argp to stop parsing here and
         return. *
      arguments->strings = &state->argv[state->next];
      state->next = state->argc;
      */

      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};
