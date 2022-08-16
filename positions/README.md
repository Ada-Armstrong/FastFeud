## Feud State Format
A simple text format to store the state of a game.
The following is the regex of the expected format of the text.

```
(s|a)(b|w)[0-9]+;[0-2]+;[0-2]+;
((((b|w)(a|n|k|m|s|w)[1-4])|.);){16}
```

The first character represents the state type (swap or action), the second
character represents which team's turn it is (black or white), the following
number is the number of quarter turns that have passed. The next two numbers
are the number of skips black and white have used in a row respectively.
Then each tile is represented by 3 characters if it is occupied or a period (.) 
if it is empty. The first character of an occupied tile is the team (black or 
white), followed by the type of piece (archer, knight, king, medic, shield, or 
wizard), and finally a single digit number representing the current hp.
