#!/bin/bash
#
readonly author="Konstantin Tretyakov"

readonly script_path=$0
readonly target_path=/usr/local/bin

readonly result_code_success=0
readonly result_code_fail=1

# internal locale codes
readonly script_locale_eng=0
readonly script_locale_rus=1
readonly script_locale_ukr=2

# localized messages: ENG, RUS, UKR
readonly message_help=(
# english
"Description
        Скрипт встановлює task1.sh в систему
        Автор: $author"
# russian
"ОПИСАНИЕ
        Скрипт встановлює task1.sh в систему
        Автор: $author"
# ukrainian
"ОПИС
        Скрипт встановлює task1.sh в систему
        Автор: $author" 
)

readonly message_perm_err=(
    "Permission denied"
    "Ошибка доступа"
    "Помилка доступу"
)

readonly message_bad_arg=(
    "Invalid key"
    "Неверный ключ"
    "Невірний ключ"
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

readonly message_file_not_found=(
    "File not found"
    "Файл не найден"
    "Файл не знайдений"
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

function CreateDirectory() {
    local path=$1

    mkdir -p "$path" &>/dev/null
    if [ $? != $result_code_success ]; then
        PrintErrorAndExit "${message_create_dir_error[$internal_locale_code]}: $path"
    fi
}

function CheckRoot() {
    if [ `id -u` != '0' ]; then
        PrintErrorAndExit "${message_perm_err[$internal_locale_code]}"
    fi
}

function InstallScript() {
    if [ '-d $target_path' == '0' ]; then
        CreateDirectory $target_path
        chmod u=rwx "$target_path"
        chmod o=rx  "$target_path"
    fi

    local task1_path="$(dirname $script_path)/task1.sh"
    if [ -f $task1_path ]; then
        cp $task1_path "$target_path/task1.sh"

        if [ $? != $result_code_success ]; then
            PrintErrorAndExit "${message_create_file_error[$internal_locale_code]}: $target_path/task1.sh"
        fi
    else
        PrintErrorAndExit "${message_file_not_found[$internal_locale_code]}: $task1_path"
    fi

    chmod u=rwx "$target_path/task1.sh"
    chmod o=r   "$target_path/task1.sh" 
}

# Entry point
    SetupInternalLocaleCode

    if (( $# != 0 )); then
        case "$1" in
            -h|--help)
                PrintHelpAndExit
                ;;
                
            *)
                PrintErrorAndExit "${message_bad_arg[$internal_locale_code]}: $1"
                ;;
        esac
    fi

    CheckRoot
    InstallScript

    exit $result_code_success
#