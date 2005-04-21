LATEX		= latex
BIBTEX		= bibtex
MAKEINDEX	= makeindex
DVIPS		= dvips
DVIPDF		= dvipdf
LATEXPDF	= latexpdf
PS2PDF		= ps2pdf

DIR_SEP		= /

.SUFFIXES: .tex .toc .dvi .aux .bbl .ind .idx $(SUFFIXES)
.PHONY: all clean veryclean bib
	
.tex.dvi:
	$(LATEX) $<
.tex.toc:
	$(LATEX) $<
.tex.idx:
	$(LATEX) $<
.tex.aux:
	$(LATEX) $<
.aux.bbl:
	$(BIBTEX) $*
.tex.ps:
	$(DVIPS) -o $<.ps $<

	
##-----------------------------------------------------------------------------

all: bsod.ps bsod.pdf

##-----------------------------------------------------------------------------

%.prepdf.ps: %.dvi
	$(DVIPS) -Ppdf -G0 $^ -o $@


%.pdf: %.prepdf.ps
	$(PS2PDF) -sPAPERSIZE=a4 -dMaxSubsetPct=100 -dCompatibilityLevel=1.2 -dSubsetFonts=true -dEmbedAllFonts=true $^ $@

%.ps: %.dvi
	$(DVIPS) -o $@ $^

bib: bsod.tex bsod.bib intro.tex design.tex operation.tex examples.tex conclusion.tex
	$(LATEX) bsod
	$(BIBTEX) bsod
	$(LATEX) bsod
	$(LATEX) bsod

bsod.dvi: bsod.tex bsod.bib intro.tex design.tex operation.tex examples.tex conclusion.tex
	$(LATEX) bsod
	$(LATEX) bsod
	$(BIBTEX) bsod
	$(LATEX) bsod
	$(LATEX) bsod


clean:
	$(RM) *.bak *.BAK *~ *.log *.ilg *.blg *.toc *.aux *.idx *.ind *.bbl *.lof

veryclean: clean
	$(RM) bsod.dvi bsod.ps bsod.pdf

viewpdf: bsod.pdf
	acroread bsod.pdf

viewps: bsod.ps
	gv bsod.ps

viewdvi: bsod.dvi
	xdvi bsod.dvi
