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


# Round-trip test with separate global/local permutations; extra stego-test
# arguments (e.g. --percent 50, --offset 4096, --invert) go in ARGN.
macro(add_stego_test_ex NAME ENCODER GLOBAL_PERM LOCAL_PERM PASSWORD)
  add_test(NAME ${NAME} COMMAND stego-test
    --test_directory
    --directory ${NAME}
    --encoder ${ENCODER}
    --global_perm ${GLOBAL_PERM}
    --local_perm ${LOCAL_PERM}
    --password ${PASSWORD}
    ${ARGN}
  )
endmacro()

# Unit test group (permutations | encoders | factories) with optional instance.
macro(add_stego_unit_test NAME GROUP)
  add_test(NAME ${NAME} COMMAND stego-unit-test ${GROUP} ${ARGN})
endmacro()
