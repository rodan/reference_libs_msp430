#!/bin/bash

#  msp430 device-specific datasheet parser 
#  collection of functions that extracts pin information from pdf datasheets
#  and generates C code used to initialize the control bits for selected pins
#  Author:          Petre Rodan <2b4eda@subdimension.ro>
#  Available from:  https://github.com/rodan/atlas430

. gen_functions.sh

# temporary output directory
output_dir='/tmp/2'

out_head()
{
    uart_id="$1"

    cat << EOF
/*
  pin setup for UCA${uart_id} subsystem. 
  this file is generated automatically based on the device datasheets
  Author:          Petre Rodan <2b4eda@subdimension.ro>
  Available from:  https://github.com/rodan/atlas430

  generated on $(date -u)
*/

#include <msp430.h>

void uart${uart_id}_pin_init(void)
{

#ifdef USE_UART${uart_id}

EOF
}

out_tail()
{
    family="$1"
    uart_id="$2"

    cat << EOF
#else
    #error "USE_UART${uart_id} was defined but pins not known in 'glue/${family}/uart${uart_id}_pin.c'"
#endif
#endif
}
EOF
}

out_head_uart_md()
{
    cat << EOF

## UART pin detection map

the automated device-specific datasheet parser has detected the following pins

Legend

code | description
---- | -------
\-   | pin not available or not detected during parsing
sh   | pin belongs to the P1-11,J ports and it shares multiple functions
pm   | default port-mapped function
de   | dedicated pin
ns   | microcontroller not supported

*Table: UART pin detection map*

uc         | uart0 | uart1 | uart2 | uart3
---------- | ----- | ----- | ----- | -----
EOF
}


gen_uart()
{
    family="$1"
    uart_id="$2"

    # output (generated) source file
    output_file="${output_dir}/${family}/uart${uart_id}_pin.c"
    mkdir -p "${output_dir}/${family}"

    rm -f "${output_file}"
    bash get_specs.sh -f "UCA${uart_id}RXD" -F "${family}" -s "uart${uart_id}" -d "${output_dir}"
    bash get_specs.sh -f "UCA${uart_id}TXD" -F "${family}" -s "uart${uart_id}" -d "${output_dir}"

    #bash get_specs.sh -f "UCA${uart_id}RXD" -T 'msp430fr5041' -s "uart${uart_id}" -d "${output_dir}"
    #bash get_specs.sh -f "UCA${uart_id}TXD" -T 'msp430fr5041' -s "uart${uart_id}" -d "${output_dir}"
    #bash get_specs.sh -f "UCA${uart_id}RXD" -T 'msp430fr2633' -s "uart${uart_id}" -d "${output_dir}"
    #bash get_specs.sh -f "UCA${uart_id}TXD" -T 'msp430fr2633' -s "uart${uart_id}" -d "${output_dir}"

    for source_in in "${output_dir}"/*_uart"${uart_id}".c; do
        source_out=${source_in//_uart${uart_id}.c/_uart${uart_id}_comb.c}
        bitwise_comb "${source_in}" "${source_out}"
    done

    out_head "${uart_id}" > "${output_file}"
    sha256sum ${output_dir}/*_uart"${uart_id}"_comb.c > "${output_dir}/uart${uart_id}.hash"
    ifdef_comb "${output_dir}/uart${uart_id}.hash" >> "${output_file}"
    out_tail "${family}" "${uart_id}" >> "${output_file}"
}

cleanup()
{
    rm -f "${output_dir}"/*_uart[0-3].c
    rm -f "${output_dir}"/*_uart[0-3]_comb.c
    rm -f "${output_dir}"/uart[0-3].hash
    sync
}

gen_code()
{
	rm -f "${output_dir}"/*_uart[0-3].log
	cleanup

	gen_uart 'MSP430FR2xx_4xx' '0'
	gen_uart 'MSP430FR2xx_4xx' '1'
	cleanup

	gen_uart 'MSP430FR5xx_6xx' '0'
	gen_uart 'MSP430FR5xx_6xx' '1'
	gen_uart 'MSP430FR5xx_6xx' '2'
	gen_uart 'MSP430FR5xx_6xx' '3'
	cleanup

	gen_uart 'MSP430F5xx_6xx' '0'
	gen_uart 'MSP430F5xx_6xx' '1'
	gen_uart 'MSP430F5xx_6xx' '2'
	gen_uart 'MSP430F5xx_6xx' '3'
	cleanup
}

get_table_elem()
{
	uc="$1"
	elem="$2"
	ret="-"

	case "${elem}" in
		0)
			ret=$(grep 'UCA0.XD pin_type' | awk '{print $NF}' | sort -u)
			;;
		1)
			ret=$(grep 'UCA1.XD pin_type' | awk '{print $NF}' | sort -u)
			;;
		2)
			ret=$(grep 'UCA2.XD pin_type' | awk '{print $NF}' | sort -u)
			;;
		3)
			ret=$(grep 'UCA3.XD pin_type' | awk '{print $NF}' | sort -u)
			;;
	esac

    [ -z "${ret}" ] && ret='-'

	echo "${ret}"
}

gen_table()
{
    ucs="$(list_ucs)"
	table="${output_dir}/uart.md"

    out_head_uart_md > "${output_dir}/uart.md"

	for uc in ${ucs}; do
		echo -n "${uc}" >> "${table}"
		for i in 0 1 2 3; do
			if [ -e "${output_dir}/${uc}_uart${i}.log" ]; then
				elem=$(get_table_elem "${uc}" "$i" < "${output_dir}/${uc}_uart${i}.log")
				echo -n " | ${elem}" >> "${table}"
			else
				echo -n ' | -' >> "${table}"
			fi
		done
		echo '' >> "${table}"
	done
}

gen_code
gen_table

# without USE_PARALLEL
# 534.40user 69.58system 8:25.61elapsed 119%CPU (0avgtext+0avgdata 15352maxresident)k
# 0inputs+776120outputs (789major+115578338minor)pagefaults 0swaps

# with USE_PARALLEL="true"
# 665.91user 88.19system 1:17.63elapsed 971%CPU (0avgtext+0avgdata 17808maxresident)k
# 0inputs+1855328outputs (12747major+100234489minor)pagefaults 0swaps

