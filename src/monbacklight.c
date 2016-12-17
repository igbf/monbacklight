#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>

#define STRING_LENGTH 5

/* Define filesystem paths */
static char* brightness_path = "/sys/class/backlight/gmux_backlight/brightness";
static char* maxbrightness_path = "/sys/class/backlight/gmux_backlight/max_brightness";

/* Define possible commands */
typedef enum
{
	get,
	set,
	get_ratio,
	set_change,
	set_percent,
	set_percent_change
} command;

/* Parse command line arguments */
command
parse_args(char** args)
{
	command action;

	if (strcmp(args[1], "get") == 0)
	{
		action = get;
		if (args[2])
		{
			if (strcmp(args[2], "ratio") == 0)
				action = get_ratio;
		}
	}
	else if (strcmp(args[1], "set") == 0)
	{
		if (!args[2])
		{
			//print_usage();
			exit(EXIT_FAILURE);
		}
		action = set;

		if (args[2][0] == '+' || args[2][0] == '-')
			action = set_change;

		if (args[2][strlen(args[2]) - 1] == '%')
		{
			args[2][strlen(args[2]) - 1] = '\0';

			switch(action)
			{
				case set_change:
					action = set_percent_change;
					break;
				default:
					action = set_percent;
			}
		}
	}

	return action;
}

/* Read brightness from filesystem */
int
get_brightness(FILE* fp)
{
	rewind(fp);

	char* brightness_str = (char*)malloc(STRING_LENGTH);
	if (!brightness_str)
		return -1;

	if (fgets(brightness_str, STRING_LENGTH, fp) == NULL)
		return -1;

	return strtol(brightness_str, NULL, 10);
}

/* Write brightness to filesystem */
void
set_brightness(FILE* fp, int value)
{
	rewind(fp);

	if (fprintf(fp, "%i", value) < 0)
		err(EXIT_FAILURE, "Error writing brightness to path");
}

/* Calculate percentage */
int
to_abs(int a, int b)
{
	return (int) ( a * b ) / 100;
}

int
main(int argc, char** args)
{
	/* The user should pass at least one argument */
	if (argc < 2)
	{
		//print_usage();
		exit(EXIT_FAILURE);
	}

	/* Parse the arguments */
	command action = parse_args(args);

	/* Get maximum brightness */
	FILE *file = fopen(maxbrightness_path, "r");
	if (!file)
		err(EXIT_FAILURE, "Could not open path: %s", maxbrightness_path);

	int max_brightness = get_brightness(file);
	fclose(file);

	/* Get current brightness */
	file = fopen(brightness_path, "r+");
	if (!file)
		err(EXIT_FAILURE, "Could not open path: %s", brightness_path);

	int brightness = get_brightness(file);
	int value;

	if (action == set || action == set_change || action == set_percent || action == set_percent_change)
	{
		value = strtol(args[2], NULL, 10);
		if (errno == EINVAL)
			err(EXIT_FAILURE, "Error parsing numerical argument");

		if (action == set_percent || action == set_percent_change)
			value = to_abs(value, max_brightness);
	}

	switch(action)
	{
		case get:
			printf("%i\n", brightness);
			break;
		case get_ratio:
			printf("%i/%i\n", brightness, max_brightness);
			break;
		case set:
		case set_percent:
			set_brightness(file, value);
			break;
		case set_change:
		case set_percent_change:
			set_brightness(file, brightness + value);
			break;
		default:
			// print_usage();
			exit(EXIT_FAILURE);
	}

	fclose(file);
	return EXIT_SUCCESS;
}
