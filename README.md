# Caatinga Boat

## Introdução

Projeto construido para demonstrar os possíveis usos de barcos rádio controlados
dentro do ambiente da caatinga no estudo de rios, lagos e açudes, visando a análise
da vida marinha e do estado de poluição da agua.

## Instalação

Para instalar o projeto deve se baixar o repositório, abrir o código do arduino e por ele
em um arduino com um módulo xbee anexado a ele, e após isso executar os comandos em um
notebook com um modulo XBEE FTDI Explorer e um controle remoto padrão XBOX 360/ONE.

```sh
git clone https://github.com/jefersonla/caatingaboat.git
cd caatingaboat
npm install
```

Para executar o projeto basta digitar `node index.js` no terminal dentro da pasta.
### Instalação Galileo

Para a instalação no Intel Galileo são necessário mais passos, visto que a plataforma não vem pronta
e não gerencia as dependências adequadamente.

```sh
bash
opkg update
opkg install nodejs nodejs-dev nodejs-npm
opkg install libusb-1.0-0 libusb-1.0-dev
opkg install linux-libc-headers-dev kernel-dev kernel-modules
cd /usr/src/kernel
make scripts
cd /home/root
git clone https://github.com/jefersonla/xpad.git
cd xpad
make
make install
echo "xpad" > /etc/modules-load.d/xpad.conf
cd /home/root
git clone https://github.com/jefersonla/caatingaboat.git
cd caatingaboat
npm install -g node-gyp node-pre-gyp nan fast-deep-equal
npm install -g json-schema-traverse json-stable-stringify co safe-buffer ms bindings
npm install -g forever-initd forever
npm install --save serialport --build-from-source
npm rebuild --save serialport --build-from-source
npm install --save gamepad
initd-forever index.js
chmod +x caatingaboat
cp caatingaboat /etc/init.d/
update-rc.d caatingaboat defaults
```

## Suporte e Desenvolvimento

Projeto desenvolvido por Jeferson Lima <@jefersonla> jefersonlimaa at dcc dot ufba dot br .
Para maiores detalhes ou sólução de problemas abrir issue aqui neste repositório, para
contatos de parceria entrar em contato por e-mail, para adições a plataforma criar pull-request.
