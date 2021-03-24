FROM acgetchell/vcpkg-image

LABEL description="CDT++ run container"

RUN git clone https://github.com/acgetchell/CDT-plusplus.git \
    && cd /CDT-plusplus/scripts \
    && ./fast-build.sh

CMD ["/bin/bash"]