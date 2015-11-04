##################################################################
######################## STEGO TESTS ##############################
##################################################################

macro(add_stego_test NAME ENCODER PERMUTATION PASSWORD)
  add_test(NAME ${NAME} COMMAND stego-test
    --test_directory
    --directory ${NAME}
    --encoder ${ENCODER}
    --permutation ${PERMUTATION}
    --password ${PASSWORD}
  )
endmacro()

