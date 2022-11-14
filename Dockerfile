FROM debian:11

USER root

WORKDIR /server

CMD [ "./avatarServer", "8888" ]
