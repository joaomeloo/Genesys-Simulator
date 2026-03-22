build_image:
	bash ./scripts/build.sh

selector:
	bash ./scripts/selector.sh

push_image:
	build_image
	source ./scripts/env.sh && docker push $GENESYS_IMAGE

run_user_gui:
	build_image
	bash ./scripts/run_docker.sh run_user_gui

run_user_shell:
	build_image
	bash ./scripts/run_docker.sh run_user_shell

run_developer_gui:
	build_image
	bash ./scripts/run_docker.sh run_developer_gui

attach_to_docker_image:
	build_image
	bash ./scripts/run_docker.sh attach_to_docker_image
