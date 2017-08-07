zed: src/*.h src/*.c
	clang -o zed src/buffer.c src/editor.c src/renderer.c src/main.c

.PHONY: test
test: src/*.c
	@clang -o zed_buffer_test src/buffer.c src/test.c src/buffer_test.c
	./zed_buffer_test
	@rm zed_buffer_test
