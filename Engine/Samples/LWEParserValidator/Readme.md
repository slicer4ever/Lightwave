Validation of XML + JSON files using Lightwave's parsers.

#JSON Test Suite
This program can be dropped into the json test suite parsers, Lightwave Json parser will not score a 100% success rate, as some obscure json formatting will be accepted/rejected.
To insert the test suite, 
1. Grab the repo from: https://github.com/nst/JSONTestSuite
2. Take the binary produced when building and place it under JSONTestSuite/parsers/Lightwave/LWEParseValidator(.exe)
3. Insert the following into run_tests.py under programs:
```

programs = {
	"Lightwave":
	{
		"url":"https://github.com/slicer4ever/lightwave.git",
		"commands":[os.path.join(PARSERS_DIR, "Lightwave/LWEParserValidator(.exe)")]
	},
	...
```
Parenthesis portion are contingent if on windows platform or linux.
4. run tests.


#XML Test Suite

Currently I am unaware of any easy drag and drop testing suite for xml the same way json works.
