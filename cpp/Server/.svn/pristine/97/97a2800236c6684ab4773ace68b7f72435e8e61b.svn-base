#!/bin/sh	    
#\
exec tclsh "$0" ${1+"$@"}

if {$argc != 1} {
    puts "Usage: optable.tcl sqlFile"
    exit 1
}

set defFile [lindex $argv 0]

set headFile "table.h"
#           define           C       appendix           SQL 
set TypeMap(serial)    [list "int"      ""            "serial"           ]
set TypeMap(character) [list "char"     ""            "character varying"]
set TypeMap(integer)   [list "int"      ""            "integer"          ]
set TypeMap(text)      [list "char*"    ""            "text"             ]
set TypeMap(float)     [list "double"   ""            "numeric(5,2)"     ]
set TypeMap(timestamp) [list "char"     "\[23\]"      "timestamp with time zone"]




if {[catch {open $defFile "r"} fpDef]} {
    puts "open $defFile failed"
    exit 1
}


if {[catch {open $headFile "w+"} fpHead]} {
    puts "open $headFile failed"
    exit 1
}


set number 0
set currentTable  ""
set comment ""

set validTypes [array names TypeMap]
while {![eof $fpDef]} {
    gets $fpDef line

    incr number
    set line [string trim $line]
    if {[string equal $line ""]} {
	continue
    }

    puts $line

    if {[regexp "^#.*" $line dummy]} {
	if {[regexp "^#define.*" $line dummy]} {
	    append comment "$line\n"
	}
	continue
    }
    
    if {[regexp "^--.*" $line dummy]} {
	    continue
    }
    if {[regexp "^SET.*" $line dummy]} {
	    continue
    }
    if {[regexp "^REVOKE.*" $line dummy]} {
	    continue
    }
    if {[regexp "^GRANT.*" $line dummy]} {
	    continue
    }
    if {[regexp "^COPY.*" $line dummy]} {
	    continue
    }
    if {[regexp "^CREATE UNIQUE.*" $line dummy]} {
	    continue
    }
    if {[regexp "^ALTER TABLE.*" $line dummy]} {
	    continue
    }
    if {[regexp ".*ADD CONSTRAINT.*" $line dummy]} {
	    continue
    }
    if {[regexp "^SELECT.*" $line dummy]} {
	    continue
    }
    if {[regexp "^COMMENT.*" $line dummy]} {
	    continue
    }
    
    set items $line
#    set nr    [llength $items]  
    if {[string match -nocase "CREATE TABLE*" $line]} {
	set currentTable [lindex $items 2]
	regsub "\"" $currentTable currentTable
	puts "get table $currentTable"
	lappend tableList $currentTable
        set Table(${currentTable},fields) ""
	set Comment(${currentTable}) $comment
	set comment "" 
    } else {
	if {[string match -nocase "Constraint*" $line]} {
	    continue
	    set keys [lindex $line 4]
	    regsub "\\("  $keys "" keys
	    regsub "\\)"  $keys "" keys
	    regsub "\""   $keys "" keys
	    set keys [split $keys ","]
	    set Table(${currentTable},key) $keys
	} else {
	    set field ""
	    if {[regexp \
	    {\"?([a-zA-Z0-9]+)\"? (.*) varying\((.*)\).*} \
	    $line dummy field type length]} {
		puts "===========$length"
		set type "${type}\(${length}\)"
	    } elseif {[regexp \
	    {\"?([a-zA-Z0-9]+)\"? (.*),.*} \
	    $line dummy field type]} {

	    } elseif {[regexp \
	    {\"?([a-zA-Z0-9]+)\"? (.*).*} \
	    $line dummy field type]} {
	    }


	    if {$field == ""} {
		continue
	    }

	    puts "----------$field  $type "
#	    set field [lindex $items 0]
#	    set type  [lindex $items 1]
	    
	    if {[string match $validTypes $type] == -1} {
		puts "invalid type defined at line $number"
		exit 1
	    }
	    

	    lappend  Table(${currentTable},fields) $field

	    set Table(${currentTable},field,${field}) $type
	}
    }
}

parray Table
close $fpDef


#Generate file table.h
regsub {\.} $headFile {_} macro
set macro [string toupper $macro]
puts $fpHead "#ifndef __$macro"
puts $fpHead "#define __$macro"

foreach table $tableList {
    puts $fpHead "$Comment(${table})"
    puts $fpHead "struct $table \{"
    set fields $Table($table,fields)
    foreach field $fields {
	set fieldDesc $Table(${table},field,${field})
	puts $fieldDesc
	set type [lindex $fieldDesc 0]
	puts $type
	set num ""
	if {[regexp {(.*)\((.*)\)} $type dummy type num]} {
	    
	}
	puts "filed=${field}, type=${type}, num=${num}"

	
	set cType    [lindex $TypeMap($type) 0]
	set appendix [lindex $TypeMap($type) 1]


	if {$num != ""} {
	    incr num
	    set desc "$cType         ${field}\[${num}\];"
	} else {
	    set desc "$cType         ${field}${appendix};"
	} 
	puts $fpHead "    $desc"
	
    }

    puts $fpHead "\};"
}
puts $fpHead "#endif"
close $fpHead



proc buildFuncs {fdh fdc table memberList Table } {
    upvar $Table UpvarTable


    puts $fdc "/* for table $table"
    puts $fdc " *"
    puts $fdc " */"

    #build header file
    puts $fdh "struct ${table}* clone_${table}\(struct ${table} *\);"
    puts $fdh "int op_${table}(Sink         *sink,"
    puts $fdh "                   struct $table *ts,"
    puts $fdh "                   int            op,"
    puts $fdh "                   const char    *filter);"
    
    puts $fdh "int get_from_${table} \(Sink *sink, SList **list, const char *filter\);"
    puts $fdh "struct $table * safe_malloc_$table\(\);"
    puts $fdh "void safe_free_${table}\(struct $table *rec\);"



    #build function 0
    puts $fdc "struct ${table}* clone_${table}(struct ${table} *rec)"
    puts $fdc "\{"
    puts $fdc "    struct ${table} *one;"
    puts $fdc "    if\(!rec\)"
    puts $fdc "        return 0;"
    puts $fdc "    one = \(struct ${table}*\)malloc\(sizeof\(struct ${table}\)\);"
    foreach field $memberList {
	set type $UpvarTable($table,field,$field)
	set type [lindex $type 0]
        regexp {(.*)\((.*)\)} $type dummy type num

	if {$type == "text"} {
	    puts $fdc "    one->${field} = strdup\(rec->${field}\);"
	} elseif {$type == "float" || $type == "integer" || $type == "serial"} {
	    puts $fdc "    one->${field} = rec->${field};"
	}  elseif {$type == "character" || $type == "timestamp"} {
	    puts $fdc "    strcpy\(one->${field}, \(char*\)rec->${field}\);"
	} else {
	    puts "invalid type $type when create clone-- function"
	    exit 1
	}
    }
    puts $fdc "    return one;"
    puts $fdc "\}"

    puts $fdc "struct $table * safe_malloc_$table\(\)"
    puts $fdc "\{"
    puts $fdc "    struct $table *one;"
    puts $fdc "    one = \(struct $table *\)malloc\(sizeof\(struct $table \)\);"
    puts $fdc "    memset\(one, 0, sizeof\(struct $table \)\);"
    puts $fdc "    return one;"
    puts $fdc "\}"


    puts $fdc "void safe_free_${table}\(struct $table *rec\)"
    puts $fdc "\{"
    puts $fdc "    if\(!rec\) return;"
    foreach field $memberList {
	set type $UpvarTable($table,field,$field)
	set type [lindex $type 0]
        regexp {(.*)\((.*)\)} $type dummy type num

	if {$type == "text"} {
	    puts $fdc "    if\(rec->$field\)"
	    puts $fdc "        free\(rec->$field\);";
	}
    }
    puts $fdc "    free\(rec\);"
    puts $fdc "\}"

    #build function 1
    puts $fdc "int op_${table}(Sink           *sink,"
    puts $fdc "                struct $table  *ts,"
    puts $fdc "                int           op,"
    puts $fdc "                const char *flt)"
    
    puts $fdc "\{"
    puts $fdc "    Result            *res;"
    puts $fdc "    char               query\[8000\];"
    puts $fdc "    const char        *filter;\n"
    puts $fdc "    int                state;"
    puts $fdc "    char               keys\[1024\], values\[8000\], temp\[600\], buffer\[10000\];"
    puts $fdc "    query\[0\]  = 0;"
    puts $fdc "    keys\[0\]   = 0;"
    puts $fdc "    values\[0\] = 0;"
    puts $fdc "    if \(flt == 0 || flt\[0\] == 0\) \{"
    puts $fdc "        filter = 0;"
    puts $fdc "    \} else \{"
    puts $fdc "        filter = flt;"
    puts $fdc "    \}\n"
    puts $fdc "    if \(op == O_GET\) \{"
    puts $fdc "        int         row;"
    puts $fdc "        const char *str;"
    puts $fdc "        if\(filter\) \{ "
    puts $fdc "            snprintf\(query, sizeof\(query\) - 1, \"select * from $table where %s\", filter\);"
    puts $fdc "        \} else \{"
    puts $fdc "            snprintf\(query, sizeof\(query\) - 1, \"select * from $table\"\);"
    puts $fdc "        \}"
    puts $fdc "        res = sink_exec\(sink, query\);"
    puts $fdc "        state = result_state\(sink, res\);"
    puts $fdc "        if\(\(state != RES_COMMAND_OK\) &&"
    puts $fdc "           \(state != RES_TUPLES_OK\)  &&"
    puts $fdc "           \(state != RES_COPY_IN\)    &&"
    puts $fdc "           \(state != RES_COPY_OUT\)\)"
    puts $fdc "        \{"
    puts $fdc "            result_clean\(sink, res\);"
    puts $fdc "            return -1;"
    puts $fdc "        \}"
    puts $fdc "        row = result_rn\(sink, res\);"
    puts $fdc "        if\(row != 1\) \{"
    puts $fdc "            result_clean\(sink, res\);"
    puts $fdc "            return -1;"
    puts $fdc "        \}"
    puts $fdc ""

    set  n 0
    set  i 0
    foreach field $memberList {
	set type $UpvarTable($table,field,$field)
	set type [lindex $type 0]
        regexp {(.*)\((.*)\)} $type dummy type num
	puts $fdc "        str = result_get\(sink, res, 0, ${n}\);"    
	if {$type == "float"} {
	    puts $fdc "        ts->${field} = atof\(str\);"
	} elseif {$type == "integer" || $type == "serial"} {
	    puts $fdc "        ts->${field} = atoi\(str\);"	    
	} elseif {$type == "text"} {
	    puts $fdc "        ts->${field} = strdup\(str\);"	    
	} elseif {$type == "timestamp" || $type == "character"} {
	    puts $fdc "        strcpy\(ts->${field}, \(char*\)result_get\(sink, res, 0, ${n}\)\);"
	} else {
	    puts "unknown filed type<$type>"
	    exit 1
	}
	incr n
    }
    puts $fdc "        result_clean\(sink, res\);"
    puts $fdc "    \} else \{"
    puts $fdc "        switch \(op\) "
    puts $fdc "        case O_CREATE: \{"
    puts $fdc ""
    set  n  1
    puts $fdc "            keys\[0\] = 0; values\[0\] = 0;"

    set length [llength $memberList]
    set idx [expr $length - 1]
    foreach field $memberList {
	set type $UpvarTable($table,field,$field)
	set type [lindex $type 0]
        regexp {(.*)\((.*)\)} $type dummy type num
	if {[lsearch $memberList $field] == $idx} {
	    set appendix ""
	} else {
	    set appendix ","
	}

	puts $fdc "            strcat\(keys, \"${field}${appendix}\"\);"
	if {$type == "integer" || $type == "serial"} {
	    puts $fdc "            sprintf\(temp, \"%d${appendix}\", ts->${field}\);"
	    puts $fdc "            strcat\(values, temp\);"
	    puts $fdc ""
	} elseif {$type == "float"} {
	    puts $fdc "            sprintf\(temp, \"%.2f${appendix}\", ts->${field}\);"
	    puts $fdc "            strcat\(values, temp\);"
	    puts $fdc ""
	} else {
	    if {$type == "text"} {
		puts $fdc "            sprintf\(temp, \"'%s'\", ts->${field} ? ts->${field} : \"\"\);"
		puts $fdc "            strcat\(values, temp\);"
	    } else {
		puts $fdc "            sprintf\(temp, \"'%s'\", ts->${field}\);"
		puts $fdc "            strcat\(values, temp\);"
	    }

	    if {$appendix != ""} {
		puts $fdc "            strcat\(values, \",\"\);"
	    }
	    puts $fdc ""
	    
	}
    }
    
    puts $fdc "            sprintf\(query, \"insert into $table \(%s\) values \(%s\)\",  keys, values\);"
    puts $fdc "            break;"
    puts $fdc "        case O_SET:"
    puts $fdc "            buffer\[0\] = 0;"

    foreach field $memberList {
	set type $UpvarTable($table,field,$field)
	set type [lindex $type 0]
        regexp {(.*)\((.*)\)} $type dummy type num

	if {$type == "integer" || $type == "serial"} {
	    puts $fdc "            sprintf\(keys, \"$field=%d,\", ts->${field}\);"
	} elseif {$type == "float"} {
	    puts $fdc "            sprintf\(keys, \"%.2f\", ts->${field}\);"
	} else {
	    if {$type == "text"} {
		puts $fdc "            sprintf\(keys, \"$field='%s',\", ts->${field} ? ts->${field} : \"\"\);"
	    } else {
		puts $fdc "            sprintf\(keys, \"$field='%s',\", ts->${field}\);"
	    }
	
	}
	puts $fdc "            strcat\(buffer, keys\);"
    }
    puts $fdc "            buffer\[strlen\(buffer\) - 1\] = 0;"

    puts $fdc "            if \(filter\) \{"
    puts $fdc "                sprintf\(query, \"update $table set %s where %s\", buffer, filter\);"
    puts $fdc "            \} else \{"
    puts $fdc "                sprintf\(query, \"update $table set %s\", buffer\);"
    puts $fdc "            \}"
    puts $fdc "            break;"
    puts $fdc "        case O_DELETE:"
    puts $fdc "            if \(filter\) \{"
    puts $fdc "                sprintf\(query, \"delete from $table where %s\", filter\);"
    puts $fdc "            \} else \{"
    puts $fdc "                strcpy\(query, \"delete from $table\");"
    puts $fdc "            \}"
    puts $fdc "            break;"
    puts $fdc "        \}";
    puts $fdc "        res = sink_exec\(sink, query\);"
    puts $fdc "        state = result_state\(sink, res\);"
    puts $fdc "        if\(\(state != RES_COMMAND_OK\) &&"
    puts $fdc "            (state != RES_TUPLES_OK)  &&"
    puts $fdc "            (state != RES_COPY_IN)  &&"
    puts $fdc "            (state != RES_COPY_OUT))"
    puts $fdc "        \{"
    puts $fdc "            result_clean\(sink, res\);"
    puts $fdc "            return -1;"
    puts $fdc "        \}"
    puts $fdc "        result_clean\(sink, res\);"
    puts $fdc "    \}"
    puts $fdc "    return 0;"
    puts $fdc "\}\n"

    #build function 2
    puts $fdc "int get_from_${table} \(Sink *sink, SList **list, const char *filter\)"
    puts $fdc "\{"
    puts $fdc "    Result              *res;"
    puts $fdc "    SList               *head = 0;"
    puts $fdc "    int                  row, state, r;"
    puts $fdc "    char                 query\[8000\];"
    puts $fdc "    if\(filter\) \{"
    puts $fdc "        sprintf\(query,  \"select * from $table where %s\", filter\);"
    puts $fdc "    \} else \{"
    puts $fdc "        sprintf\(query,  \"select * from $table\"\);"
    puts $fdc "    \}"
    puts $fdc ""
    puts $fdc "    row = 0;"
    puts $fdc "    res = sink_exec\(sink, query\);"
    puts $fdc "    state = result_state\(sink, res\);"
    puts $fdc "    if\(\(state != RES_COMMAND_OK\) &&"
    puts $fdc "       \(state != RES_TUPLES_OK\)  &&"
    puts $fdc "       \(state != RES_COPY_IN\)    &&"
    puts $fdc "       \(state != RES_COPY_OUT\)\)"
    puts $fdc "    \{"
    puts $fdc "        result_clean\(sink, res\);"
    puts $fdc "        return 0;"
    puts $fdc "    \}"
    puts $fdc "    row = result_rn\(sink, res\);"
    puts $fdc "    r = 0;"
    puts $fdc "    for \( ; r < row; r++) \{"
    puts $fdc "        const char *value;"
    puts $fdc "        struct $table *record = \(struct $table *\)malloc\(sizeof\(struct $table\)\);"
    
    for {set i 0} {$i < [llength $memberList]} {incr i} {
	set field [lindex $memberList $i]
	puts $fdc "        value = result_get\(sink, res, r, $i\);"
	set type $UpvarTable($table,field,$field)
	set type [lindex $type 0]
        regexp {(.*)\((.*)\)} $type dummy type num

	if {$type == "integer" || $type == "serial"} {
	    puts $fdc "        record->${field} = atoi\(value\);"
	} elseif {$type == "float"} {
	    puts $fdc "        record->${field} = atof\(value\);"
	} elseif {$type == "text"} {
	    puts $fdc "        record->${field} = strdup\(value\);"

	} else {
	    puts $fdc "        strcpy(record->${field}, \(char*\)value\);"
	}
    }
    puts $fdc "        head = slist_prepend\(head, record\);"
    puts $fdc "    \}"
    puts $fdc "    result_clean\(sink, res\);"
    puts $fdc "    *list = head;"
    puts $fdc "    return row;"
    puts $fdc "\}"
}

set tbFile  "table.h"
set outputH "optable.h"
set outputC "optable.c"

if {[catch {open $tbFile r} fd] } {
    puts "open $tbFile failed"
    exit 1
}

if {[catch {open $outputH w} fdh]} {
    puts "open $outputH failed"
    exit 1
}

if {[catch {open $outputC w} fdc]} {
    puts "open $outputC failed"
    exit 1
}

regsub {\.} $outputH {_} macro
set macro [string toupper $macro]
puts $fdh "#ifndef __$macro"
puts $fdh "#define __$macro"
puts $fdh "#include <db/sink.h>"
puts $fdh "#include <db/$tbFile>"
puts $fdh "#include <com/slist.h>"
puts $fdh "#include <string.h>"
puts $fdh "#include <stdlib.h>"
puts $fdh "" 
puts $fdh "#ifdef __cplusplus"
puts $fdh "extern \"C\" \{"
puts $fdh "#endif"

puts $fdc "/* operations for database access. "
puts $fdc " * _DON'T_ modify it manually.\n"
puts $fdc " * This file is generated by optable.tcl script. It contains a set of"
puts $fdc " * table access short cuts."
puts $fdc " */\n"

puts $fdc "#include <stdio.h>"
puts $fdc "#include <string.h>"
puts $fdc "#include <db/$outputH>"
puts $fdc "#include <db/sink.h>"
puts $fdc "#include <db/table.h>"


foreach table $tableList {
    set fields $Table($table,fields)
    buildFuncs $fdh $fdc $table $fields Table
}

puts  $fdh "#ifdef __cplusplus"
puts  $fdh "\}"
puts  $fdh "#endif"
puts  $fdh "#endif"
close $fdh
close $fdc
