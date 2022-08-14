## Feud State Format
A simple text format to store the state of a game.
The following is the regex of the expected format of the text.

```
(s|a)(b|w)[0-9]+;[0-9]+;[0-9]+;
((b|w)(a|n|k|m|s|w)[1-4];){16}
```
