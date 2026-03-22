FROM debian:latest

# Versão do Dockerfile
ARG VERSION=0.1.0

ENV GENESYS_USER="genesys-user"
ENV GENESYS_REPO=https://github.com/rlcancian/Genesys-Simulator

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

LABEL maintainer=$GENESYS_USER \
    url=$GENESYS_REPO \
    name="docker-genesys" \
    version=$VERSION

# Instala todas as bibliotecas necessárias
RUN echo "Installing packages" && \
    apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get --assume-yes install \
        build-essential \
        curl \
        git \
        libclang1 \
        libxcb-cursor0 \
        libgl1-mesa-dri \
        mesa-utils\
        xdg-utils \
        qtcreator \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        graphviz \
        libgtest-dev \
        cmake && \
    apt-get autoremove --assume-yes && \
    apt-get clean && \
    rm --force --recursive /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN echo "Preparing GitHub CLI" && \
    type -p curl >/dev/null || (apt-get update && apt-get install curl -y) && \
    curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg && \
    chmod go+r /usr/share/keyrings/githubcli-archive-keyring.gpg && \
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | tee /etc/apt/sources.list.d/github-cli.list > /dev/null && \
    apt-get update && \
    apt-get install gh -y

# Clona o GenESyS
RUN if [ "$REMOTE" = "1" ]; then \
        echo "Cloning GeNESyS" && \
        git clone -b $GENESYS_BRANCH $GENESYS_REPO $GENESYS_ROOT; \
    else \
        echo "Using local repository"; \
    fi
    
# Copia o script de inicialização para dentro do container
COPY scripts/env.sh /usr/local/bin/env.sh 
COPY config.sh /usr/local/bin/config.sh 
COPY scripts/entrypoint.sh /usr/local/bin/entrypoint.sh 
RUN chmod +x /usr/local/bin/entrypoint.sh

# Executa o script de inicialização
ENTRYPOINT ["bash", "/usr/local/bin/entrypoint.sh"]
