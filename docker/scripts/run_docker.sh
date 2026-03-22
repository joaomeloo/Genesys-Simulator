source ./scripts/env.sh

MODE=$1

COMMON_ARGS=(
    -e DISPLAY=$DISPLAY
    --device /dev/dri
    --env-file config.sh
    --name genesys --rm -ti --net=host --ipc=host
    -v /tmp/.X11-unix:/tmp/.X11-unix
)

if docker image inspect "$GENESYS_IMAGE" >/dev/null 2>&1; then
    xhost +local:root

    if [[ $REMOTE == 1 ]]; then
        docker run "${COMMON_ARGS[@]}" \
            $GENESYS_IMAGE $MODE
    else
        docker run "${COMMON_ARGS[@]}" \
            -v $GENESYS_PROJECT:$GENESYS_ROOT \
            $GENESYS_IMAGE $MODE
    fi
else
    echo "Build not executed"
fi
