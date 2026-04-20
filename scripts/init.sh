#!/bin/bash

set -e

DESTOK_APP_DIR=/home/vboxuser/.local/share/applications/
REPO_URL=https://github.com/rlcancian/Genesys-Simulator.git
REPO_DIR=/home/vboxuser/Documents
BRANCH=currentStable

# Executavel do Genesys QT GUI
GENESYS_GUI_APP_DISPLAY_NAME=GenESySQt
GENESYS_GUI_APP_EXEC=genesys_qt_gui_application
BUILD_GENESYS_GUI_APP_PATH=$REPO_DIR
BUILD_GENESYS_GUI_APP_PATH+=/Genesys-Simulator/build/gui-app/source/applications/gui/qt/GenesysQtGUI/
BUILD_GENESYS_GUI_APP_PATH+=$GENESYS_GUI_APP_EXEC

# Icone
ICON_NAME=genesysico.gif
PROJECT_ICON_PATH=$REPO_DIR
PROJECT_ICON_PATH+=/Genesys-Simulator/source/applications/gui/qt/GenesysQtGUI/resources/icons/
PROJECT_ICON_PATH+=$ICON_NAME

# Instalacao
INSTALL_DIR=/usr/bin/
ICON_DIR=/usr/share/icons/


cd $REPO_DIR
if [ ! -d Genesys-Simulator ] then
    git clone -b $BRANCH $REPO_URL
fi
cd Genesys-Simulator

# Verifica se é um repo git
git rev-parse --is-inside-work-tree > /dev/null 2>&1 || exit 1

git fetch origin

LOCAL=$(git rev-parse HEAD)
REMOTE=$(git rev-parse origin/$BRANCH)

if [ "$LOCAL" != "$REMOTE" ]; then

    gxmessage -buttons Sim:0,Não:1 \
              -default Não \
              "Há uma nova versão do GenESyS disponível. Deseja atualizar?"

    if [ $? -eq 0 ]; then
        # Aviso de loading
        gxmessage -buttons "" -timeout 9999 "Atualizando..." &
        PID=$!

        git pull origin "$BRANCH"

        cmake --preset gui-app
        cmake --build --preset gui-app
        cp -a $BUILD_GENESYS_GUI_APP_PATH $INSTALL_DIR
        cp -a $PROJECT_ICON_PATH $ICON_DIR

        # Adiciona genesys qt gui ao menu iniciar
        printf '%s\n' \
            "[Desktop Entry]" \
            "Name=$GENESYS_GUI_APP_DISPLAY_NAME" \
            "Exec=$INSTALL_DIR/$GENESYS_GUI_APP_EXEC" \
            "Icon=$ICON_DIR/$ICON_NAME" \
            "Type=Application" \
            "Terminal=false" \
            "Categories=Development;" \
            > "$DESTOK_APP_DIR$GENESYS_GUI_APP_DISPLAY_NAME.desktop"
        
        rm -rf $REPO_DIR/Genesys-Simulator/build/

        # qtcreator já está no menu iniciar por padrão

        # TODO Adicionar web-app quando compilar corretamente
        # TODO Adicionar inicialização do servidor quando disponível

        kill $PID 2>/dev/null

        gxmessage "Repositório atualizado."
    else
        gxmessage "Atualização cancelada."
    fi

else
    gxmessage "GenESyS está em sua versão mais atual!"
fi