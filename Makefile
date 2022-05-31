compiler = clang++
optimization = O0

mainfile=main
mainobject=$(mainfile).o
files=parser.o

outfile=$(mainfile).exe

run: $(mainobject)
	$(compiler) -o $(outfile) $(mainfile).o
	rm $(mainfile).o
	./$(outfile)

clean:
	rm $(mainfile).o
	rm $(outfile)

$(mainobject):
	clear
	$(compiler) -c $(mainfile).cpp -$(optimization)
