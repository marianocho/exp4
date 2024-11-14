# Dockerfile para configurar o ambiente do XV6
FROM ubuntu:20.04

# Configurações para evitar prompts de interação durante a instalação de pacotes
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Sao_Paulo

# Instalar as dependências necessárias para compilar e rodar o XV6
RUN apt-get update && \
    apt-get install -y build-essential qemu-system-x86 git tzdata \
    gcc-multilib g++-multilib libgcc-9-dev && \
    rm -rf /var/lib/apt/lists/*

# Definir o diretório de trabalho
WORKDIR /xv6

# Copiar o código fonte do XV6 para dentro do container
COPY . .

# Remover flags de compilação que podem causar problemas de compatibilidade (se necessário)
# Vamos substituir as flags `-fno-pie` e `-no-pie` no Makefile do XV6
RUN sed -i 's/-fno-pie -no-pie//g' Makefile

# Compilar o XV6
RUN make clean && make

# Definir o container para iniciar com um shell interativo
CMD ["/bin/bash"]
