#! /bin/sh

optimise="n"		# either "y" or "n"
input="phrases.lang"
output="phrases.h"

########################################################################

echo -n "# Transforming $input -> $output ... "

cat > $output <<EOL
//
//	$output: Automatically generated - do not edit!
//		(edit $input instead, and re-run init-phrases.sh)
//

static const char *phrases_raw =
EOL

if test "$optimise" = "y"; then
	cat $input | grep -v "^#" | grep -v "^\s*$" \
		| sed 's/"/\\"/g' | sed 's/\\\\"/\\\\\\"/g' \
		| sed 's/^/"/' | sed 's/$/\\n"/' \
		>> $output
else
	# Identical, except don't remove blank lines or comments
	cat $input \
		| sed 's/"/\\"/g' | sed 's/\\\\"/\\\\\\"/g' \
		| sed 's/^/"/' | sed 's/$/\\n"/' \
		>> $output
fi

cat >> $output <<EOL
;

// The End.
EOL

echo "done!"
