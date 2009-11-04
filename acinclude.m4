AC_DEFUN(AC_CHECK_LIB_LOC,
[AC_MSG_CHECKING([for lib$2 in $1])
ac_lib_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
AC_CACHE_VAL(ac_cv_lib_loc_$ac_lib_var,
[ac_save_LIBS="$LIBS"
LIBS="-L$1 -l$2 $6 $LIBS"
AC_TRY_LINK(
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $3();
],
            [$3()],
            eval "ac_cv_lib_loc_$ac_lib_var=yes",
            eval "ac_cv_lib_loc_$ac_lib_var=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo '$ac_cv_lib_loc_'$ac_lib_var`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$4], ,
[LIBS="-L$1 -l$2 $LIBS"
], [$4])
else
  AC_MSG_RESULT(no)
ifelse([$5], , , [$5
])dnl
fi
])
