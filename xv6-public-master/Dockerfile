# Dockerfile para o XV6
FROM ubuntu:20.04

# Configurações para evitar prompts de interação
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Sao_Paulo

# Instalar as dependências necessárias para compilar e rodar o XV6
RUN apt-get update && \
    apt-get install -y build-essential qemu-system-x86 git tzdata && \
    rm -rf /var/lib/apt/lists/*

# Definir o diretório de trabalho
WORKDIR /xv6

# Copiar o código fonte do XV6 para dentro do container
COPY . .

# Compilar o XV6 (opcional: mantenha essa linha para evitar recompilar ao rodar)
RUN make clean && make

# Definir o container para iniciar com um shell interativo
CMD ["/bin/bash"]