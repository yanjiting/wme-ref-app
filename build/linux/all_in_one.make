
ifdef MAKE_DEPTH
  include $(MAKE_DEPTH)/macros.make
  include $(MAKE_DEPTH)/rules.make
else
  all clean: echo_error
endif # MAKE_DEPTH

echo_error:
	@echo "Error, Makefile must define MAKE_DEPTH."

