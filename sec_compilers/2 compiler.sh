#!/bin/bash

# PL/0 Compiler+Interpreter
# v1.0 Beta
# 2009-04-07 (last mod: 2009-04-09)
#
# String processing in Bash is...SLOOOOOOOOOOOOOOOOOOOOOOW!

ch=
sym=
id=
line=
st_name=
st_kind=
st_lvl=
st_addr=
st_val=
rwords=( BEGIN CALL CASE CEND CONST DO DOWNTO ELSE END FOR IF ODD OF PROCEDURE REPEAT THEN TO UNTIL VAR WHILE WRITE WRITELN )
ops=( + - \* / \( \) = , . \<\> \< \> \<= \>= \; : := )
ops_names=( PLUS MINUS TIMES SLASH LPAREN RPAREN EQUAL COMMA PERIOD NE LT GT LTE GTE SEMICOLON COLON BECOMES )
st_types=( CONSTANT VARIABLE PROCEDURE )
blank=0
value=
instr=( LIT OPR LOD STO CAL INT JMP JPC )
code_i=
code_l=
code_a=
norw=22		# number of reserved words
txmax=100	# max identifiers in symbol table
nmax=14		# max size of numbers
al=10		# max size of identifiers

block() {
	local tx=$1
	local lev=$2
	local dx=
	local tx0=
	local codeinx0=

	# list code for current block
	listcode() {
		local i=$codeinx0
		for (( ; i<$codeinx ; i++ )); do
			echo $i ${code_i[$i]} ${code_l[$i]} ${code_a[$i]}
		done
		echo
	}

	# find position of IDENT in symbol table 
	position() {
		st_name[0]=$id
		local i=$1
		for (( ; i>=0 ; i-- )); do
			if [ "${st_name[$i]}" == "$id" ]; then
				return $i
			fi
		done

		return 0
	}

	# enter IDENT in symbol table
	enter() {
		# we should use txmax to keep symbol table no greater than max number of identifiers
		local loc=$(($2 + 1))
		st_name[$loc]=$id
		st_kind[$loc]=$1
		if [ "$1" == CONSTANT ]; then
			st_val[$loc]=$value
		elif [ "$1" == VARIABLE ]; then
			st_lvl[$loc]=$lev
			st_addr[$loc]=$dx
			dx=$(($dx + 1))
		elif [ "$1" == PROCEDURE ]; then
			st_lvl[$loc]=$lev
		fi
		return $loc
	}

	# print the symbol table (for debugging purposes)
	print_st() {
		echo -e "\tNAME\tKIND\t\tLEVEL\tADDRESS\tVALUE"
		for (( i=0 ; i<=$tx ; i++ )); do
			echo -e "$i\t${st_name[$i]}\t${st_kind[$i]}\t${st_lvl[$i]}\t${st_addr[$i]}\t${st_val[$i]}"
		done
	}

	# declares IDENT as a CONSTANT
	constdeclaration() {
		if [ "$sym" != "IDENT" ]; then
			error 1
		fi
		getsym
		if [ "$sym" != EQUAL ]; then
			error 2
		fi
		getsym
		if [ "$sym" != NUMBER ]; then
			error 3
		fi
		getsym
		enter CONSTANT $tx
	}

	# declares IDENT as a VARIABLE
	vardeclaration() {
		if [ "$sym" != IDENT ]; then
			error 1
		fi
		getsym
		enter VARIABLE $tx
	}

	statement() {
		local tx=$1
		local cx1=
		local cx2=

		condition() {
			local tx=$1
			local savesym=

			if [ "$sym" == ODD ]; then
				getsym
				expression $tx
				gen OPR 0 6
			else
				expression $tx
				if [ "$sym" != EQUAL ] && [ "$sym" != NE ] && [ "$sym" != LT ] && [ "$sym" != GT ] && [ "$sym" != LTE ] && [ "$sym" != GTE ]; then
					error 10
				fi
				savesym=$sym
				getsym
				expression $tx
				case "$savesym" in
					EQUAL) gen OPR 0 8
					;;
					NE) gen OPR 0 9
					;;
					LT) gen OPR 0 10
					;;
					GTE) gen OPR 0 11
					;;
					GT) gen OPR 0 12
					;;
					LTE) gen OPR 0 13
					;;
				esac
			fi
		}

		expression() {
			local tx=$1
			local savesym=

			term() {
				local tx=$1
				local savesym=

				factor() {
					local tx=$1

					if [ "$sym" == IDENT ]; then
						position $tx
						local i=$?
						if [ "$i" == 0 ]; then
							error 13
						fi
						case "${st_kind[$i]}" in
							VARIABLE) gen LOD $(($lev - ${st_lvl[$i]})) ${st_addr[$i]}
							;;
							CONSTANT) gen LIT 0 ${st_val[$i]}
							;;
							PROCEDURE) error 14
							;;
						esac
						getsym
					elif [ "$sym" == NUMBER ]; then
						gen LIT 0 $value
						getsym
					elif [ "$sym" == LPAREN ]; then
						getsym
						expression $tx
						if [ "$sym" != RPAREN ]; then
							error 11
						fi
						getsym
					else
						error 12
					fi
				}

				# TERM
				factor $tx
				while [ "$sym" == TIMES ] || [ "$sym" == SLASH ]; do
					savesym=$sym
					getsym
					factor $tx
					if [ "$savesym" == TIMES ]; then
						gen OPR 0 4
					else
						gen OPR 0 5
					fi
				done
			}

			# EXPRESSION
			if [ "$sym" == PLUS ] || [ "$sym" == MINUS ]; then
				savesym=$sym
				getsym
			fi
			term $tx
			if [ "$savesym" == MINUS ]; then
				gen OPR 0 1
			fi
			while [ "$sym" == PLUS ] || [ "$sym" == MINUS ]; do
				savesym=$sym
				getsym
				term $tx
				if [ "$savesym" == PLUS ]; then
					gen OPR 0 2
				else
					gen OPR 0 3
				fi
			done
		}

		# STATEMENT
		if [ "$sym" == IDENT ]; then
			position $tx
			local i=$?
			local j=$i
			if [ "$i" == 0 ]; then
				error 13
			fi
			if [ "${st_kind[$i]}" != VARIABLE ]; then
				error 14
			fi
			getsym
			if [ "$sym" != BECOMES ]; then
				error 6
			fi
			getsym
			expression $tx
			gen STO $(($lev - ${st_lvl[$j]})) ${st_addr[$j]}
		elif [ "$sym" == CALL ]; then
			getsym
			if [ "$sym" != IDENT ]; then
				error 1
			fi
			position $tx
			local i=$?
			if [ "$i" == 0 ]; then
				error 13
			fi
			if [ "${st_kind[$i]}" != PROCEDURE ]; then
				error 15
			fi
			gen CAL $(($lev - ${st_lvl[$i]})) ${st_addr[$i]}
			getsym
		elif [ "$sym" == BEGIN ]; then
			getsym
			statement $tx
			while [ "$sym" == SEMICOLON ]; do
				getsym
				statement $tx
			done
			if [ "$sym" != END ]; then
				error 7
			fi
			getsym
		elif [ "$sym" == IF ]; then
			getsym
			condition $tx
			cx1=$codeinx
			gen JPC 0 0
			if [ "$sym" != THEN ]; then
				error 8
			fi
			getsym
			statement $tx
			code_a[$cx1]=$codeinx
			##### PLACE CODE FOR ELSE HERE #####
		elif [ "$sym" == WHILE ]; then
			getsym
			cx1=$codeinx
			condition $tx
			cx2=$codeinx
			gen JPC 0 0
			if [ "$sym" != DO ]; then
				error 9
			fi
			getsym
			statement $tx
			gen JMP 0 $cx1
			code_a[$cx2]=$codeinx
#		elif [ "$sym" == REPEAT ]; then
			##### PLACE CODE FOR REPEAT-UNTIL HERE AND UNCOMMENT elif ABOVE #####
#		elif [ "$sym" == WRITE ]; then
			##### PLACE CODE FOR WRITE HERE AND UNCOMMENT elif ABOVE #####
#		elif [ "$sym" == WRITELN ]; then
			##### PLACE CODE FOR WRITELN HERE AND UNCOMMENT elif ABOVE #####
#		elif [ "$sym" == FOR ]; then
			##### PLACE CODE FOR FOR-DO HERE AND UNCOMMENT elif ABOVE #####
#		elif [ "$sym" == CASE ]; then
			##### PLACE CODE FOR CASE-CEND HERE AND UNCOMMENT elif ABOVE #####
		fi
	}

	# BLOCK
	dx=3
	tx0=$tx
	st_addr[$tx]=$codeinx
	gen JMP 0 0

	while [ "$sym" == CONST ] || [ "$sym" == VAR ] || [ "$sym" == PROCEDURE ]; do
		if [ "$sym" == CONST ]; then
			getsym
			constdeclaration $tx
			tx=$?
			while [ "$sym" == COMMA ]; do
				getsym
				constdeclaration $tx
				tx=$?
			done
			if [ "$sym" != SEMICOLON ]; then
				error 4
			fi
			getsym
		fi
		if [ "$sym" == VAR ]; then
			getsym
			vardeclaration $tx
			tx=$?
			while [ "$sym" == COMMA ]; do
				getsym;
				vardeclaration $tx
				tx=$?
			done
			if [ "$sym" != SEMICOLON ]; then
				error 4
			fi
			getsym
		fi
		while [ "$sym" == PROCEDURE ]; do
			getsym
			if [ "$sym" != IDENT ]; then
				error 1
			fi
			enter PROCEDURE $tx
			tx=$?
			getsym
			if [ "$sym" != SEMICOLON ]; then
				error 4
			fi
			getsym
			block $tx $(($lev + 1))
			if [ "$sym" != SEMICOLON ]; then
				error 4
			fi
			getsym
		done
	done
	code_a[${st_addr[$tx0]}]=$codeinx
	st_addr[$tx0]=$codeinx
	codeinx0=$codeinx
	gen INT 0 $dx
#	print_st
	statement $tx
	gen OPR 0 0
	listcode
}

error() {
	case $1 in
		1) echo Identifier expected
		;;
		2) echo EQUAL \(=\) expected
		;;
		3) echo NUMBER expected
		;;
		4) echo SEMICOLON \(\;\) expected
		;;
		5) echo PERIOD \(.\) expected
		;;
		6) echo BECOMES \(:=\) expected
		;;
		7) echo END expected
		;;
		8) echo THEN expected
		;;
		9) echo DO expected
		;;
		10) echo Relational operator expected
		;;
		11) echo RPAREN \(\)\) expected
		;;
		12) echo An expression cannot begin with this symbol
		;;
		13) echo Undeclared identifier
		;;
		14) echo Assignment to CONSTANT or PROCEDURE not allowed
		;;
		15) echo CALL must be followed by a PROCEDURE
		;;
		16) echo UNTIL expected
		;;
		17) echo LPAREN \(\(\) expected
		;;
		18) echo TO or DOWNTO expected
		;;
		19) echo NUMBER or CONSTANT expected
		;;
		20) echo COLON \(:\) expected
		;;
		21) echo OF expected
		;;
		22) echo CEND expected
		;;
		*) echo Unknown error!
		;;
	esac
	exit
}

getsym() {
	local tok=

	# matches a token to an operator
	match() {
		local tok=$1
		local i=0
		for (( ; i<"${#ops[@]}" ; i++ )); do
			if [ "$tok" == "${ops[$i]}" ]; then
				echo "${ops_names[$i]}"
				break
			fi
		done
	}

	getchar() {
		getline() {
			read
			line=${REPLY}
			line=`echo "$line" | sed 's/\t/    /g' | sed 's/\r//g' | sed 's/\n//g'`
			echo "$line"
			if [ -z "$line" ]; then
				let "blank+=1"
				if [ $blank -gt 5 ]; then
					exit
				fi
			else
				blank=0
			fi
		}

		# GETCHAR
		if [ -z "$line" ]; then
			getline
		fi
		ch=`echo "$line" | sed 's/\(.\).*/\1/'`
		line=`echo "$line" | sed 's/.\(.*\)/\1/'`
	}

	# GETSYM
	while [ -z "$ch" ] || [ "$ch" == " " ]; do
		getchar
	done
	if [ `echo "$ch" | grep '[A-Z]'` ]; then
		tok=$ch
		getchar
		while [ `echo "$ch" | grep '[A-Z0-9]'` ]; do
			if [ "${#tok}" == $al ]; then
				error
			fi

			tok=$tok$ch
			if [ -z "$line" ]; then
				getchar
				break
			fi
			getchar
		done
		sym=IDENT
		# we could use norw as sentinel... but why?
		for i in "${rwords[@]}"; do
			if [ "$tok" == "$i" ]; then
				sym=$tok
				break
			fi
		done
		id=$tok
	elif [ `echo "$ch" | grep '[0-9]'` ]; then
		tok=$ch
		getchar
		while [ `echo "$ch" | grep '[0-9]'` ]; do
			if [ "${#tok}" == $nmax ]; then
				error
			fi

			tok=$tok$ch
			getchar
		done
		sym=NUMBER
		value=$tok
	else
		if [ "$ch" == ":" ]; then
			tok=$ch
			getchar
			if [ "$ch" == "=" ]; then
				tok=$tok$ch
				getchar
			fi
		elif [ "$ch" == ">" ]; then
			tok=$ch
			getchar
			if [ "$ch" == "=" ]; then
				tok=$tok$ch
				getchar
			fi
		elif [ "$ch" == "<" ]; then
			tok=$ch
			getchar
			if [ "$ch" == "=" ] || [ "$ch" == ">" ]; then
				tok=$tok$ch
				getchar
			fi
		else
			tok=$ch
			getchar
		fi

		sym=$(match "$tok")
	fi
}

# generates p-Code
gen() {
	code_i[codeinx]=$1
	code_l[codeinx]=$2
	code_a[codeinx]=$3
	codeinx=$(($codeinx + 1))
}

# the p-Machine (p-Code interpreter)
interpret() {
	p=
	b=
	t=
	s=
	i=
	l=
	a=

	# finds base index "lv" levels down
	base() {
		local lv=$1
		local b1=$b

		while [ "$lv" -gt 0 ]; do
			b1=${s[$b1]}
			lv=$(($lv - 1))
		done
		return $b1
	}

	# prints the stack (for debugging purposes)
	print_stack() {
		st=
		for (( i=$t ; i>0 ; i-- )); do
			st="$i: ${s[$i]}"
			if [ "$b" == "$i" ]; then
				st="$st\tB"
			fi
			if [ "$t" == "$i" ]; then
				st="$st\tT"
			fi
			echo -e "$st"
			st=
		done
	}

	# INTERPRET
	echo Start PL/0
	t=0
	b=1
	p=0
	s[1]=0
	s[2]=0
	s[3]=0

	while true; do
		i=${code_i[$p]}
		l=${code_l[$p]}
		a=${code_a[$p]}
#		echo $p: $i $l $a
		p=$(($p + 1))

		case $i in
			LIT)	t=$(($t + 1))
				s[$t]=$a
			;;
			OPR)	case $a in
					0)	t=$(($b - 1))
						p=${s[$(($t + 3))]}
						b=${s[$(($t + 2))]}
					;;
					1)	ta=${s[$t]}
						ta=$(($ta * -1))
						s[$t]=$ta
					;;
					2)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						s[$t]=$(($ta + $tb))
					;;
					3)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						s[$t]=$(($ta - $tb))
					;;
					4)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						s[$t]=$(($ta * $tb))
					;;
					5)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						s[$t]=$(($ta / $tb))
					;;
					6)	ta=${s[$t]}
						ta=$(($ta % 2))
						if [ "$ta" == 0 ]; then
							s[$t]=0
						else
							s[$t]=1
						fi
					;;
					8)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						if [ "$ta" == "$tb" ]; then
							s[$t]=1
						else
							s[$t]=0
						fi
					;;
					9)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						if [ "$ta" -ne "$tb" ]; then
							s[$t]=1
						else
							s[$t]=0
						fi
					;;
					10)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						if [ "$ta" -lt "$tb" ]; then
							s[$t]=1
						else
							s[$t]=0
						fi
					;;
					11)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						if [ "$ta" -ge "$tb" ]; then
							s[$t]=1
						else
							s[$t]=0
						fi
					;;
					12)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						if [ "$ta" -gt "$tb" ]; then
							s[$t]=1
						else
							s[$t]=0
						fi
					;;
					13)	tb=${s[$t]}
						t=$(($t - 1))
						ta=${s[$t]}
						if [ "$ta" -le "$tb" ]; then
							s[$t]=1
						else
							s[$t]=0
						fi
					;;
					14)	echo -n "${s[$t]} "
						t=$(($t - 1))
					;;
					15)	echo
					;;
					*)	echo "!!HOUSTON, WE HAVE A PROBLEM!!"
					;;
				esac
			;;
			LOD)	t=$(($t + 1))
				base $l
				val=$?
				s[$t]=${s[$(($val + $a))]}
			;;
			STO)	base $l
				s[$(($? + $a))]=${s[$t]}
				t=$(($t - 1))
			;;
			CAL)	base $l
				s[$(($t + 1))]=$?
				s[$(($t + 2))]=$b
				s[$(($t + 3))]=$p
				b=$(($t + 1))
				p=$a
			;;
			INT)	t=$(($t + $a))
			;;
			JMP)	p=$a
			;;
			JPC)	if [ "${s[$t]}" == $l ]; then
					p=$a
				fi
				t=$(($t - 1))
			;;
		esac

#		print_stack
		if [ "$p" == 0 ]; then
			break
		fi
	done

	echo End PL/0
}

######
# MAIN
######

getsym
block 0 0
if [ "$sym" != PERIOD ]; then
	error 5
fi
echo Successful compilation!
echo
interpret
