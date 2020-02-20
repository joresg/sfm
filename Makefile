# dmenu - dynamic menu
# See LICENSE file for copyright and license details.

include config.mk

SRC = sfm.c
OBJ = $(SRC:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $<

$(OBJ): config.mk

sfm: sfm.o
	$(CC) -o $@ sfm.o $(LDFLAGS)

clean:
	rm -f sfm $(OBJ)

.PHONY: all options clean
