source config.sh

# Nome do usuário
USER=$(whoami)

if [[ $REMOTE == 1 ]]; then
  GENESYS_ROOT=$GENESYS_REMOTE_ROOT
else
  GENESYS_ROOT=$GENESYS_LOCAL_ROOT
fi

if ! command -v docker &> /dev/null; then
    echo "Docker not installed"
    exit 1
fi

sudo chown $USER /var/run/docker.sock

