SRC=./src/*.c
TESTSRC=./src/test/*.c
TARGET=lab1-file-transfer

main:
	gcc -o $(TARGET) $(SRC)
	@echo ""
	@echo "========================================================================"
	@echo ""
	@echo "USAGE"
	@echo ""
	@echo "FILE TRANSFER sender   : ./$(TARGET) send [tcp/udp] [host] [port] [filename]"
	@echo "FILE TRANSFER receiver : ./$(TARGET) recv [tcp/udp] [host] [port]"
	@echo ""
	@echo "========================================================================"
	@echo ""

test:
	gcc -o $@ $(TESTSRC)
test-clean:
	rm test
clean:
	rm $(TARGET)
