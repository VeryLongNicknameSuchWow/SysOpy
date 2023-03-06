REPL_PATH=../zad2
INPUT_PATH=input.txt

test_all: test_static test_shared test_dynamic

test_static:
	make -C $(REPL_PATH) static
	$(REPL_PATH)/wc_repl_static < $(INPUT_PATH) > results_static.txt
	make -C $(REPL_PATH) clean

test_shared:
	make -C $(REPL_PATH) shared
	$(REPL_PATH)/wc_repl_shared < $(INPUT_PATH) > results_shared.txt
	make -C $(REPL_PATH) clean

test_dynamic:
	make -C $(REPL_PATH) dynamic
	$(REPL_PATH)/wc_repl_dynamic < $(INPUT_PATH) > results_dynamic.txt
	make -C $(REPL_PATH) clean

clean:
	rm -f results*.txt