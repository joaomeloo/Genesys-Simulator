#!/bin/bash
if [[ $REMOTE == 1 ]]; then
    GENESYS_ROOT=$GENESYS_REMOTE_ROOT
    echo "Executing from remote"
    git -C $GENESYS_ROOT fetch --all
    git -C $GENESYS_ROOT reset --hard origin/$GENESYS_BRANCH
else
    GENESYS_ROOT=$GENESYS_LOCAL_ROOT
    echo "Executing form local"
fi

if [[ $1 == "run_user_gui" ]]; then
    cp -a $GENESYS_ROOT/autoloadplugins.txt $GENESYS_ROOT/$GENESYS_GUI_SUBPATH
    sed -i 's/runGraphicalUserInterface = false/runGraphicalUserInterface = true/g' $GENESYS_ROOT/source/applications/TraitsApp.h
    cd $GENESYS_ROOT/$GENESYS_GUI_SUBPATH
    qmake6
    make -j6
    $GENESYS_ROOT/$GENESYS_GUI_SUBPATH_BIN
elif [[ $1 == "run_user_shell" ]]; then
    cp -a $GENESYS_ROOT/autoloadplugins.txt $GENESYS_ROOT/$GENESYS_NOGUI_SUBPATH
    cd $GENESYS_ROOT/$GENESYS_NOGUI_SUBPATH
    make -j6
    $GENESYS_ROOT/$GENESYS_NOGUI_SUBPATH_BIN
elif [[ $1 == "run_developer_gui" ]]; then
    cp -a $GENESYS_ROOT/autoloadplugins.txt $GENESYS_ROOT/$GENESYS_GUI_SUBPATH
    /usr/bin/qtcreator $GENESYS_ROOT/$GENESYS_PROJECT_SUBPATH
elif [[ $1 == "attach_to_docker_image" ]]; then
    /bin/bash
fi

