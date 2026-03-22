source ./scripts/env.sh

while true
do
read -p "
Choose an option:
.............................
1. Run GenESyS GUI
2. Run GenESyS Shell
3. Run developer GUI
4. Attach to docker image
5. Exit
> " input

    if [ "$input" == "5" ]; then
        break
    fi

    case "$input" in
        "1")
        bash ./scripts/build.sh
        bash ./scripts/run_docker.sh run_user_gui
        ;;
        "2")
        bash ./scripts/build.sh
        bash ./scripts/run_docker.sh run_user_shell
        ;;
        "3")
        bash ./scripts/build.sh
        bash ./scripts/run_docker.sh run_developer_gui
        ;;
        "4")
        bash ./scripts/build.sh
        bash ./scripts/run_docker.sh attach_to_docker_image
        ;;
        *)
        echo -e "\nInvalid option."
        ;;
    esac
done
