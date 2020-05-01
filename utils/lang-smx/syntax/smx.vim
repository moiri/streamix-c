" Vim syntax file
" Language: Streamix
" Maintainer: Simon Maurer
" Latest Revision: 8 October 2019

if exists("b:current_syntax")
  finish
endif

syn keyword smxCollection up down left right side
syn keyword smxMode in out
syn keyword smxType box wrapper net
syn keyword smxKeyword pure static extern
syn keyword smxKeyword decoupled coupled open tt tb tf dynamic
syn keyword smxOperator tt tb tf connect

syntax match smxPotionOperator "\v\|"
syntax match smxPotionOperator "\v\."
syntax match smxPotionOperator "\v\:"
syntax match smxPotionOperator "\v\!"

syntax region  smxComment        start=/\/\// end=/$/ extend keepend
syntax region  smxComment        start=/\/\*/  end=/\*\// fold extend keepend

hi def link smxType             Type
hi def link smxComment          Comment
hi def link smxKeyword          Keyword
hi def link smxMode             Boolean
hi def link smxOperator         Operator
hi def link smxPotionOperator   Operator
hi def link smxCollection       Special
