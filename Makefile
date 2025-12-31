CC = gcc
CFLAGS = -I./include -I../rk_hw_base/include -I/usr/include/rockchip -fPIC -O2 -Wall
LDFLAGS = -shared -L../rk_hw_base/lib -lrk_hw_base -lva -lva-drm

TARGET = lib/rockchip_drv_video.so
SRCS = src/driver.c src/surface.c src/context.c src/encoder.c src/buffer.c src/stubs.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p lib
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET) lib
