#!/bin/bash
#
readonly author="Konstantin Tretyakov"
# default path
readonly default_output=~/bash/task1.out
readonly script_path=$0

readonly result_code_success=0
readonly result_code_fail=1

# internal locale codes
readonly script_locale_eng=0
readonly script_locale_rus=1
readonly script_locale_ukr=2

# localized messages: ENG, RUS, UKR
readonly message_help=(
# english
"USAGE
        $script_path [-h|--help] [-n num] [file]
DESCRIPTION
        Script collects basic information about equipment, OS and network interface configurations.
        Author: $author
ARGUMENTS
        -h|--help       Print help
        -n num          num - number of files with results 
                        0 < num < 10001
        file            Output path
                        Default: "$default_output"
"
# russian
"ОПИСАНИЕ
        Скрипт собирает основную информацию об оборудовании, ОС и конфигурации сетевых устройств.
        Автор: $author
ИСПОЛЬЗОВАНИЕ
        $script_path [-h|--help] [-n num] [file]
АРГУМЕНТЫ
        -h|--help       Показать помощь
        -n num          num - количество файлов с результатами
                        0 < num < 10001
        file            Путь, по которому необходимо записать результат
                        По-умолчанию: "$default_output"
"
# ukrainian
"ОПИС
        Скрипт збирає базову інформацію про обладнання, ОС та конфігурацію мережевих інтерфейсів.
        Автор: $author
ВИКОРИСТОВУВАННЯ
        $script_path [-h|--help] [-n num] [file]
АРГУМЕНТИ
        -h|--help       Показати допомогу
        -n num          num - кількість файлів з результатами
                        0 < num < 10001
        file            Шлях, за яким необхідно записати результат
                        За замовчуванням: "$default_output"
" 
)

readonly message_perm_err=(
    "Permission denied"
    "Ошибка доступа"
    "Помилка доступу"
)

readonly message_create_dir_error=(
    "Could not create directory"
    "Не удалось создать директорию"
    "Не вдалось створити директорію"
)

readonly message_create_file_error=(
    "Could not create file"
    "Не удалось создать файл"
    "Не вдалось створити файл"
)

readonly message_move_file_error=(
    "Could not replace/rename file"
    "Не удалось переместить/переименовать файл"
    "Не вдалось перемістити/перейменувати файл"
)

readonly message_file_limit_exceeded=(
    "File limit exceeded per day"
    "Превышен лимит файлов в день"
    "Перевищено ліміт файлів на добу"
)

function SetupInternalLocaleCode() {
    local lang_str=$LANG
    local system_locale=${lang_str:0:2}
    
    case $system_locale in
        ru)
            internal_locale_code=$script_locale_rus
            ;;
        uk)
            internal_locale_code=$script_locale_ukr
            ;;
        *)
            internal_locale_code=$script_locale_eng
            ;;
    esac
}

function PrintHelpAndExit() {
    echo "${message_help[$internal_locale_code]}"
    exit $result_code_success
}

function PrintErrorAndExit() {
    echo "[Error]: $1" >&2
    exit $result_code_fail
}

function CheckRoot() {
    if [ `id -u` != '0' ]; then
        PrintErrorAndExit "${message_perm_err[$internal_locale_code]}"
    fi
}

function CreateDirectory() {
    local path=$1

    mkdir -p "$path" &>/dev/null
    if [ $? != $result_code_success ]; then
        PrintErrorAndExit "${message_create_dir_error[$internal_locale_code]}: $path"
    fi
}

function CreateFile() {
    local path=$1

    touch "$path" &>/dev/null

    if [ $? != $result_code_success ]; then
        PrintErrorAndExit "${message_create_file_error[$internal_locale_code]}: $path"
    fi
}

function MoveFile() {
    local old_path=$1
    local new_path=$2

    mv $old_path $new_path &>/dev/null

    if [ $? != $result_code_success ]; then
        PrintErrorAndExit "${message_move_file_error[$internal_locale_code]}: $old_path -> $new_path"
    fi
}

# ----------------------------------------------------------------
filepath=$default_output
num=0
# ----------------------------------------------------------------

function PrepareOutput() {
    local dir=$(dirname $filepath)
    local filename=$(basename $filepath)
    local datestamp=$(date +%Y%m%d)

    if [ -d $dir ]; then
        cd $dir
        filelist=$(ls -1 "$filename-$datestamp-"* 2>/dev/null)
        old_files_found=$(( $? == $result_code_success ))

        if [ -f $filename ]; then
            next_file_num_str="0000"
            
            if (( $old_files_found == 1 )); then
                old_files_count=$(echo "$filelist" | wc -l)
                last_file=$(echo "$filelist" | awk '{print $NF}')
                next_file_num_int=$(echo $last_file | awk 'BEGIN{FS="-"} {print $NF+1}')

                if (( $next_file_num_int > 9999 )); then
                    # file limit exceeded or last_file_num not valid
                    PrintErrorAndExit "${message_file_limit_exceeded[$internal_locale_code]} > 10000"
                fi

                next_file_num_str=$(printf "%04i" $next_file_num_int)
            fi
            
            MoveFile "$filename" "$filename-$datestamp-$next_file_num_str"
            (( ++old_files_count ))
        fi
        
        if (( $num > 0 )); then
            if (( $old_files_found == 1 )); then
                local temp=$(( $old_files_count - $num ))

                if (( $temp > 0 )); then 
                    rm $(echo "$filelist" | head -n $temp )
                fi
            fi
        fi
        cd $OLDPWD
    else
        CreateDirectory "$dir"
        chmod a=rwx "$dir"
    fi

    CreateFile "$filepath"
    chmod a=rw "$filepath"
}

function CollectHWInfo() {  
    local cpu=$(dmidecode -s processor-version | sed -e 's/ *$//')

    local temp=$(dmidecode -t memory | grep  Size: | grep -v "No Module Installed")
    local memory_size=$(echo "$temp" | awk '{ size += $2 } END { printf "%i MB\n", size }')

    local mb_manufacturer=$(dmidecode -s baseboard-manufacturer)
    local mb_product_name=$(dmidecode -s baseboard-product-name)
    local sys_serial_number=$(dmidecode -s system-serial-number)

    echo "---- Hardware ----"                                       >>$filepath
    echo "CPU: $cpu"                                                >>$filepath
    echo "RAM: $memory_size"                                        >>$filepath
    echo "Motherboard: \"$mb_manufacturer\", \"$mb_product_name\""  >>$filepath
    echo "System Serial Number: $sys_serial_number"                 >>$filepath
}

function CollectOSInfo() {
    local distrib_name=$(lsb_release -d -s)                    
    local kernel_version=$(uname -r)

    local fs_name=$(mount | grep 'on / ' | awk '{print $1}')
    local fs_info=$(sudo dumpe2fs $fs_name 2>/dev/null)
    local os_install_date=$(echo "$fs_info" | grep 'Filesystem created:' | cut -c 27-)
    
    local uptime_s=$(uptime -p | awk \
    '{
        if (NF > 3)
            { printf "%02i:%02i", $2, $4 } 
        else 
            { printf "00:%02i", $2 }
    }')

    local processes_count=$(ps -A --no-headers | wc -l) 
    local users=$(who | wc -l)

    echo "---- System ----"                     >>$filepath
    echo "OS Distribution: \"$distrib_name\""   >>$filepath
    echo "Kernel version: $kernel_version"      >>$filepath
    echo "Installation date: $os_install_date"  >>$filepath
    echo "Hostname: $(hostname)"                >>$filepath
    echo "Uptime: $uptime_s"                    >>$filepath
    echo "Processes running: $processes_count"  >>$filepath
    echo "User logged in: $users"               >>$filepath
}

function CollectNetworkInfo() {
    local adapters_count=$(ip -f inet address | grep -c inet)

    if (( $adapters_count > 0 )); then
        local adapters_info=$(ip -f inet address | grep inet | awk '{print $NF ": " $2}')
    else
        local adapters_info="-/-"
    fi

    echo "---- Network ----" >>$filepath
    echo "$adapters_info"    >>$filepath
}

function CollectInfo() {
    echo "Date: $(LANG=en_EN date '+%a, %d %b %Y %T %z')" >>$filepath

    CollectHWInfo
    CollectOSInfo
    CollectNetworkInfo

    echo "----\"EOF\"----" >>$filepath
}

# Entry Point
    SetupInternalLocaleCode

    # Parse command line arguments
    while (( $# != 0 ))
    do
        case "$1" in
            -h|--help)
                PrintHelpAndExit
                ;;

            -n)
                if (( $# == 1 )); then 
                    PrintErrorAndExit "num not found"
                fi

                num=$(( $2 + 0 ))
                if (( $num < 1 || $num > 10000 )); then
                    PrintErrorAndExit "num not valid"
                fi

                shift 2
                ;;
                
            *)
                filepath="$1"
                shift 1
                ;;
        esac
    done

    CheckRoot
    PrepareOutput
    CollectInfo

    exit $result_code_success
#