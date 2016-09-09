#variables
assignment_num ?= 1.02
name = talbert_james
main_target = dungeons
folder_name = $(name).assignment-$(assignment_num)

gcc_flags = -ggdb -Wall -Werror -lm 

#Top-level targets
dungeons: dungeons.a main.c save.a
	gcc $(gcc_flags) -c -o main.o main.c 
	gcc $(gcc_flags) -o $@ main.o save.a dungeons.a 

all: dungeons

save.o: save.c
	gcc $(gcc_flags) -c -o $@ save.c

save.a: save.o room_list.o
	ld -r -o $@ save.o room_list.o 
#tile_queue.o

pqueue_test: int_pqueue.o priority_queue.h pqueue_test.c
	gcc $(gcc_flags) pqueue_test.c int_pqueue.o -o $@

list_test: int_list.o list_test.c
	gcc $(gcc_flags) list_test.c int_list.o -o $@

examples: clean $(main_target)
	./$(main_target) >> examples
	sleep 1
	./$(main_target) >> examples
	sleep 1
	./$(main_target) >> examples
	sleep 1
	./$(main_target) >> examples
	sleep 1
	./$(main_target) --round >> examples
	sleep 1
	./$(main_target) --round >> examples
	sleep 1
	./$(main_target) --round >> examples
	sleep 1
	./$(main_target) --round >> examples
	less examples

clean: clean_general
	rm -f *.map
	rm -f *_test
	rm -f examples
	cd dungeon; make -s clean
	cd data_structures; make -s clean

#dependancies
dungeons.a: dungeon/*.c dungeon/*.c path_pqueue.o tile_list.o tile_queue.o tile_pqueue.o
	cd dungeon; make -s dungeons.a;
	ld -r -o $@ dungeon/dungeons.a path_pqueue.o tile_list.o tile_queue.o tile_pqueue.o

%_list.o: data_structures/list.c
	cd data_structures; make -s list TYPE=$*; cp $*_list.o ../$*_list.o
	cp data_structures/$*_list.o $*_list.o

%_queue.o: data_structures/queue.c�
	cd data_structures; make -s queue TYPE=$*; cp $*_queue.o ../$*_queue.o
	cp data_structures/$*_queue.o $*_queue.o

%_pqueue.o: data_structures/priority_queue.c
	cd data_structures; make -s pqueue TYPE=$*; cp $*_pqueue.o ../$*_pqueue.o
	cp data_structures/$*_pqueue.o $*_pqueue.o

tile_queue.o:
	cd data_structures; make -s queue TYPE=tile_t* NAME=tile HEADER=dungeon/dungeons.h;
	cp data_structures/tile_queue.o tile_queue.o

tile_pqueue.o:
	cd data_structures; make -s pqueue TYPE=tile_t* NAME=tile HEADER=dungeon/dungeons.h;
	cp data_structures/tile_pqueue.o tile_pqueue.o

tile_list.o:
	cd data_structures; make -s list TYPE=tile_t* NAME=tile HEADER=dungeon/dungeons.h;
	cp data_structures/tile_list.o tile_list.o

room_list.o:
	cd data_structures; make -s list TYPE=rectangle_t NAME=room HEADER=dungeon/coordinates.h;
	cp data_structures/room_list.o room_list.o

path_pqueue.o: dungeon/dungeons_private.h data_structures/priority_queue.c
	cd data_structures; make -s pqueue TYPE=tile_dijkstra_t* NAME=path HEADER=dungeon/dungeons_private.h; cp path_pqueue.o ../path_pqueue.o


#Common targets
clean_general:
        # object files/binaries
	rm -f *.o $(main_target) *.o.i *.a
        # emacs buffers
	rm -f *~ \#*\#
        # old submission data 
	rm -f $(name).*.tar.gz
	rm -r -f ../$(folder_name)
	rm -r -f $(folder_name)
	rm -r -f test_folder
        # .directory file that keeps popping up
	rm -f .directory
        # example folder
	rm -r -f examples

submission: $(main_target) clean Makefile CHANGELOG README
	cp -R . ../$(folder_name)
	tar cvfz $(folder_name).tar.gz ../$(folder_name)
	rm -r -f ../$(folder_name)

test: submission
	gzip -dc $(folder_name).tar.gz | tar xvf -
	cp -f -r $(folder_name) test_folder
	rm -r -f $(folder_name)
	cd test_folder; make examples
	rm -fr test_folder

##shortcut
sub: submission