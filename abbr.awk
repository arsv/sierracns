/^O[iI]D|^CMD|^SUB/ { print "#line " FNR " \"" FILENAME "\""; print $0 }
