# Embedded steno engine.

Embedded steno engine.

This repository contains the raw steno engine, without any bindings.

For pico-sdk bindings and build instructions, see
https://github.com/jthlim/javelin-steno-rp2040.

For the online firmware builder tool, go to http://lim.au/#/software/javelin-steno

# Supported Commands

## `=retro_transform:<n>:<format>`

Transforms the prior `n` translations using `format`.

Format has the following special keys:

- `%s` or `%o`: Stroke list
- `%l` or `%d`: Lookup results
- `%t`: Text results

Example usage:
With `SPHRA*EUPB` defined as:

```
  =retro_transform:2:%s: "%l" -> "%t"
```

Assuming default main.json, then:

```
T/T/T/T/SPHRA*EUPB: it it `T/T`: "it it" -> "it it"
T/PHRAOES/-S/SPHRA*EUPB: `T/PHRAOES/-S`: "it please {^s}" -> "it pleases"
```

## `=transform:<format>`

Transforms the string, substituting template values in `format`

- `%0` - `%63`: Template value
- `%%`: '%' character

Example usage:

```
  =transform:Hello %0
```

Modifiers supported:

- `%u`: Upper case
- `%l`: Lower case
- `%t`: Title case
- `%c`: Capitalize

Example usage:

```
  =transform:Hello %u0
```

## `=set_value:<template value>:<n>`

Sets `template value` to the last `n` lookup translations. The value adopted is
removed from input to indicate what was set.

Example usage:

```
  =set_value:0:2
```

This sets template value 0 to the last 2 lookups.

Suggested bindings:

```
  "SR*FR": "=set_value:0:1",
  "SR*FR/SR*FR": "=set_value:0:2",
  "SR*FR/SR*FR/SR*FR": "=set_value:0:3",
  "SR*FR/SR*FR/SR*FR/SR*FR": "=set_value:0:4",
  "SR*PB": "=set_value:1:1",
  "SR*PB/SR*PB": "=set_value:1:2",
  "SR*PB/SR*PB/SR*PB": "=set_value:1:3",
  "SR*PB/SR*PB/SR*PB/SR*PB": "=set_value:1:4",
  "SR*LG": "=set_value:2:1",
  "SR*LG/SR*LG": "=set_value:2:2",
  "SR*LG/SR*LG/SR*LG": "=set_value:2:3",
  "SR*LG/SR*LG/SR*LG/SR*LG": "=set_value:2:4",
  "SR*TS": "=set_value:3:1",
  "SR*TS/SR*TS": "=set_value:3:2",
  "SR*TS/SR*TS/SR*TS": "=set_value:3:3",
  "SR*TS/SR*TS/SR*TS/SR*TS": "=set_value:3:4",
  "SR*DZ": "=set_value:4:1",
  "SR*DZ/SR*DZ": "=set_value:4:2",
  "SR*DZ/SR*DZ/SR*DZ": "=set_value:4:3",
  "SR*DZ/SR*DZ/SR*DZ/SR*DZ": "=set_value:4:4",
```

## `{:add_translation}`

Interactive prompt to add translation.

Example usage:

```
  {:add_translation}
```

Providing a parameter will cause only the strokes to be prompted. This can be
useful in combination with =transform and template values.

```
  =transform:{:add_translation:\n\nHello %0}
```

## `{:console:}`

Starts interactive console mode.

Example usage:

```
  Console>{^ ^}{:console}
```

## `{:console:<cmd>}`

Runs a console command. The list of commands can be found using `help` in the
[console tool](https://lim.au/#/software/javelin-steno-tools)

Example usage:

```
  {:console:set_space_position after}
```

## `{:disable_dictionary:<dictionary_name>}`

Disables a dictionary.

Example usage:

```
  {:disable_dictionary:main.json}
```

## `{:enable_dictionary:<dictionary_name>}`

Enables a dictionary.

Example usage:

```
  {:enable_dictionary:main.json}
```

## `{:host_layout:<layout_name>}`

Sets the layout of the host computer.

Supported values are determined at firmware build time.

```
  {:host_layout:us_windows_alt}
```

## `{:reset_state}`

Resets the state of the steno engine.

This removes all stroke history, and sets the next stroke to output without a
leading space, and in default (lower) case.

Example usage:

```
  {:reset_state}
```

## `{:retro_capitalise:<nth_word>}`

Converts the prior `nth_word` word to capital.

Example usage:
Text: `This is a test`

```
  {:retro_capitalise:3}
```

Output: `This Is a test`

## `{:retro_double_quotes:<n_words>}`

Surrounds the prior `n_words` with double quotes.

Example usage:
Text: `This is a test`

```
  {:retro_double_quotes:2}
```

Output: `This is "a test"`

## `{:retro_lower:<n_words>}`

Lower cases the prior `n_words`.

Example usage:
Text: `THIS IS A TEST`

```
  {:retro_lower:2}
```

Output: `THIS IS a test`

## `{:retro_replace_space:<n>:<replacement>}`

Replaces the prior `n` spaces with `<replacement>`.

Example usage:
Text: `This is a test`

```
  {:retro_replace_space:2:_}
```

Output: `This is_a_test`

## `{:retro_single_quotes:<n_words>}`

Surrounds the prior `n_words` with single quotes.

Example usage:
Text: `This is a test`

```
  {:retro_single_quotes:2}
```

Output: `This is 'a test'`

## `{:retro_surround:<n_words>:<prefix>:<suffix>}`

Surrounds the prior `n_words` with `prefix` and `suffix`.

Example usage:
Text: `This is a test`

```
  {:retro_surround:2:[:]}
```

Output: `This is [a test]`

## `{:retro_title:<n_words>}`

Converts the prior `n_words` to title case.

Example usage:
Text: `This is a test`

```
  {:retro_title:3}
```

Output: `This Is A Test`

## `{:retro_upper:<n_words>}`

Converts the prior `n_words` to upper case.

Example usage:
Text: `This is a test`

```
  {:retro_upper:3}
```

Output: `This IS A TEST`

## `{:set_case:<case>}`

Sets the case for output.

Supported values:

- normal
- lower
- upper
- title

Example usage:

```
  {:set_case:title}
```

## `{:set_space:<separator>}`

Use `separator` between words.

Example usage:

The following will give "snake case" words:

```
  {:set_space:_}
```

## `{:stitch:<text>:<delimiter?>}`

Use `delimiter` between text, if the previous text was also a stitch.

Example usage:

- Stitched capital letter alphabet:

  `{:stitch:A}{:stitch:B}` → `A-B`

- Joining repeated words

  `{:stitch:ha:}` → `ha`

  `{:stitch:ha:}{:stitch:ha:}{:stitch:ha:}` → `hahaha`

- Custom delimiter

  `{:stitch:lol:o}` → `lol`

  `{:stitch:lol:o}{:stitch:lol:o}{:stitch:lol:o}` → `lololololol`

## `{:stitch_last_word:<n>:<delimiter>}`

Use `delimiter` between the last `n` words.

Example usage:
Text: `This is a test`

```
  {:stitch:3:-}
```

Output: `This i-s a t-e-s-t`

## `{:disable_dictionary:<dictionary_name>}`

Toggles a dictionary between enabled and disabled.

Example usage:

```
  {:toggle_dictionary:main.json}
```

# Contributions

This project is a snapshot of internal repositories and is not accepting any
pull requests.

# Terms

This code is distributed under [PolyForm Noncommercial license](LICENSE.txt).
For commercial use, please [contact](mailto:jeff@lim.au) me.
