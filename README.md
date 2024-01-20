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

## `{:add_translation}`

Interactive prompt to add translation.

Example usage:

```
  {:add_translation}
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

## `{:keyboard_layout:<layout_name>}`

Sets the keyboard layout of the host computer.

Supported values:

- qwerty
- dvorak
- colemak
- workman
- qmglwy

Example usage:

```
  {:keyboard_layout:dvorak}
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

## `{:disable_dictionary:<dictionary_name>}`

Toggles a dictionary between enabled and disabled.

Example usage:

```
  {:toggle_dictionary:main.json}
```

## `{:unicode:<unicode_mode>}`

Sets the unicode output mode.

Supported values:

- none
- macos_us
- macos_hex
- windows_alt
- windows_hex
- linux_ibus

Example usage:

```
  {:unicode:windows_hex}
```

# Contributions

Note that contributions are not currently being accepted until I get around
to setting up a Contributor License Agreement.

# Terms

This code is distributed under [PolyForm Noncommercial license](LICENSE.txt).
For commercial use, please [contact](mailto:jeff@lim.au) me.
