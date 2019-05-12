#!/bin/bash

INPUT_FILE="input.in"
OUTPUT_FILE="output.out"
ERROR_FILE="error.out"
VALGRIND_FILE="v.out"
EXE_FILE="tema3"
IN_DIR="in"
REF_DIR="ref"
OUT_DIR="out"
VALGRIND="valgrind --leak-check=full -q --log-file=${VALGRIND_FILE}"
max_points=85
total_tests=$(ls $IN_DIR | wc -l)
bonus_points=20
result_points=0
valgrind_points=0

# set DEBUG on YES to have outputs and CLEAN_OUT on NO to keep them
DEBUG="YES"
CLEAN_OUT="NO"

# ------------------------- USEFUL FUNCTIONS ------------------------- #

print_header()
{
	header="${1}"
	header_len=${#header}
	printf "\n"
	if [ $header_len -lt 71 ]; then
		padding=$(((53 - $header_len) / 2))
		for ((i = 0; i < $padding; i++)); do
			printf " "
		done
	fi
	printf "= %s =\n\n" "${header}"
}

test_do_fail()
{
	printf " ${red}failed${reset}  [0/%d]\n" "${1}"
	if test "x$EXIT_IF_FAIL" = "x1"; then
		exit 1
	fi
}

test_do_pass()
{
	printf " ${green}passed${reset}  [%d/%d]\n" "${1}" "${1}"
	((result_points+=${1}))
}

valgrind_ok()
{
	((valgrind_points++))
	printf "  VALGRIND ..... ${green}OK${reset}\n"
}

valgrind_failed()
{
	printf "  VALGRIND ..... ${red}FAILED${reset}\n"
}

valgrind_failed_sf()
{
	printf "  VALGRIND ..... ${red}SEG FAULT${reset}\n"
}

valgrind_bonus_msg()
{
	if [ $verify_vg -eq ${1} ]; then
		printf "${yellow}VALGRIND BONUS${reset} ........ ${green}passed${reset}  [%d/%d]\n\n" ${2} ${2}
		((result_points+=${2}))
	else
		printf "${yellow}VALGRIND BONUS${reset} ........ ${red}failed${reset}  [0/%d]\n\n" ${2}
	fi
}

set_up()
{
	cp "$IN_DIR/$1" "$INPUT_FILE"
}

tear_down()
{
	rm -f "${INPUT_FILE}" "${OUTPUT_FILE}" "${ERROR_FILE}" "${VALGRIND_FILE}" &> /dev/null
}

test_function()
{
	test_file="$(basename $1)"
	test_nr=$3
	test_points=$2
	ref_file_out="${REF_DIR}/test${test_nr}.out"
	ref_file_err="${REF_DIR}/test${test_nr}.err"
	
	set_up "${test_file}"

	$VALGRIND "./"$EXE_FILE < $INPUT_FILE > $OUTPUT_FILE 2> $ERROR_FILE

	if [ "$(cat "${ERROR_FILE}" | grep -o "Segmentation fault")" == "Segmentation fault" ]; then
		printf "[test%02d.in] ..........." ${test_nr}
		test_do_fail $test_points
		valgrind_failed_sf ${test_nr}
	
	else
		diff -Z "${OUTPUT_FILE}" "${ref_file_out}" > /dev/null
		cmp_out=$?
		
		diff -Z "${ERROR_FILE}" "${ref_file_err}" > /dev/null
		cmp_err=$?

		if [ $cmp_out -eq 0 ] && [ $cmp_err -eq 0 ]; then
			printf "[test%02d.in] ..........." ${test_nr}
			test_do_pass $test_points

			if [[ -s ${VALGRIND_FILE} ]]; then
				valgrind_failed ${test_nr}
			else
				valgrind_ok ${test_nr}
			fi
		else
			printf "[test%02d.in] ..........." ${test_nr}
			test_do_fail $test_points
			valgrind_failed ${test_nr}
		fi
	fi

	if [ $DEBUG == "YES" ]; then
		cp $OUTPUT_FILE $OUT_DIR/test$test_nr.out
		cp $ERROR_FILE $OUT_DIR/test$test_nr.err
	fi

	tear_down
}

init_function()
{
	make build

	if [ $? -ne 0 ]; then
		echo -e "Building failed!"
		exit 1
	fi

	if [ ! -e ${IN_DIR} ]; then
		echo "No in directory!"
		exit 1
	fi
	if [ ! -e ${REF_DIR} ]; then
		echo "No ref directory!"
		exit 1
	fi

	if [ $DEBUG == "YES" ] && [ ! -d "$OUT_DIR" ]; then
		mkdir $OUT_DIR
	fi
}

finish()
{
	((result_points+=$(($bonus_points*$valgrind_points/$total_tests))))

	printf "\nTOTAL ...................... [%d/%d]\n" $result_points $max_points
	if [ $result_points -eq 105 ]; then
		echo -e " \n      ${yellow}Congrats, you rock! :D${reset}"
	fi
	echo ""
}

clean_out_function()
{
	make clean
	rm -f *.o &> /dev/null

	if [ $CLEAN_OUT == "YES" ]; then
		rm -rf $OUT_DIR
	fi
}

# -------------------- TESTING ---------------------- #

init_function

print_header "TEMA 3 - Linux File System"

#Testing

test_points=(7 7 7 8 8 8 10 10 10 10)
test_nr=0

for t in ${IN_DIR}/*
do
	test_function $t ${test_points[$test_nr]} $test_nr
	((test_nr++))
done

finish
clean_out_function
