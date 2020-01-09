#ifndef INC_Parser_hpp__
#define INC_Parser_hpp__

/**
 * <b>SOFTWARE RIGHTS</b>
 * <p>
 * ANTLR 2.6.0 MageLang Insitute, 1999
 * <p>
 * We reserve no legal rights to the ANTLR--it is fully in the
 * public domain. An individual or company may do whatever
 * they wish with source code distributed with ANTLR or the
 * code generated by ANTLR, including the incorporation of
 * ANTLR, or its output, into commerical software.
 * <p>
 * We encourage users to develop software with ANTLR. However,
 * we do ask that credit is given to us for developing
 * ANTLR. By "credit", we mean that if you use ANTLR or
 * incorporate any source code into one of your programs
 * (commercial product, research project, or otherwise) that
 * you acknowledge this fact somewhere in the documentation,
 * research report, etc... If you like ANTLR and have
 * developed a nice tool with the output, please mention that
 * you developed it using ANTLR. In addition, we ask that the
 * headers remain intact in our source code. As long as these
 * guidelines are kept, we expect to continue enhancing this
 * system and expect to make other tools available as they are
 * completed.
 * <p>
 * The ANTLR gang:
 * @version ANTLR 2.6.0 MageLang Insitute, 1999
 * @author Terence Parr, <a href=http://www.MageLang.com>MageLang Institute</a>
 * @author <br>John Lilley, <a href=http://www.Empathy.com>Empathy Software</a>
 * @author <br><a href="mailto:pete@yamuna.demon.co.uk">Pete Wells</a>
 */

#include "antlr/antlr_export.h"
#include "antlr/config.hpp"
#include "antlr/BitSet.hpp"
#include "antlr/TokenBuffer.hpp"
#include "antlr/RecognitionException.hpp"
#include "antlr/ASTFactory.hpp"
#include "antlr/ParserSharedInputState.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

/**A generic ANTLR parser (LL(k) for k>=1) containing a bunch of
 * utility routines useful at any lookahead depth.  We distinguish between
 * the LL(1) and LL(k) parsers because of efficiency.  This may not be
 * necessary in the near future.
 *
 * Each parser object contains the state of the parse including a lookahead
 * cache (the form of which is determined by the subclass), whether or
 * not the parser is in guess mode, where tokens come from, etc...
 *
 * <p>
 * During <b>guess</b> mode, the current lookahead token(s) and token type(s)
 * cache must be saved because the token stream may not have been informed
 * to save the token (via <tt>mark</tt>) before the <tt>try</tt> block.
 * Guessing is started by:
 * <ol>
 * <li>saving the lookahead cache.
 * <li>marking the current position in the TokenBuffer.
 * <li>increasing the guessing level.
 * </ol>
 *
 * After guessing, the parser state is restored by:
 * <ol>
 * <li>restoring the lookahead cache.
 * <li>rewinding the TokenBuffer.
 * <li>decreasing the guessing level.
 * </ol>
 *
 * @see antlr.Token
 * @see antlr.TokenBuffer
 * @see antlr.TokenStream
 * @see antlr.LL1Parser
 * @see antlr.LLkParser
 */

extern bool DEBUG_PARSER;

class ANTLR_EXPORT Parser {
protected:
	ParserSharedInputState inputState;

	/** Nesting level of registered handlers */
	// int exceptionLevel;

	/** Table of token type to token names */
	ANTLR_USE_NAMESPACE(std)vector<ANTLR_USE_NAMESPACE(std)string> tokenNames;
	/** AST return value for a rule is squirreled away here */
	RefAST returnAST;
	/** AST support code; parser and treeparser delegate to this object */
	ASTFactory astFactory;

//	Parser();

	Parser(TokenBuffer& input_);
	Parser(TokenBuffer* input_);

	Parser(const ParserSharedInputState& state);

public:
	virtual ~Parser();

protected:
	void setTokenNames(const char** tokenNames_);

public:
	/**Get another token object from the token stream */
	virtual void consume()=0;

	/** Consume tokens until one matches the given token */
	void consumeUntil(int tokenType);

	/** Consume tokens until one matches the given token set */
	void consumeUntil(const BitSet& set);

	/** Get the AST return value squirreled away in the parser */
	RefAST getAST();

	ASTFactory& getASTFactory();

	ANTLR_USE_NAMESPACE(std)string getFilename() const;

	virtual ParserSharedInputState getInputState() const;

	ANTLR_USE_NAMESPACE(std)string getTokenName(int num) const;
	ANTLR_USE_NAMESPACE(std)vector<ANTLR_USE_NAMESPACE(std)string> getTokenNames() const;

	/** Return the token type of the ith token of lookahead where i=1
	 * is the current token being examined by the parser (i.e., it
	 * has not been matched yet).
	 */
	virtual int LA(int i)=0;

	/**Return the ith token of lookahead */
	virtual RefToken LT(int i)=0;

	// Forwarded to TokenBuffer
	virtual int mark();

	/**Make sure current lookahead symbol matches token type <tt>t</tt>.
	 * Throw an exception upon mismatch, which is catch by either the
	 * error handler or by the syntactic predicate.
	 */
	void match(int t);

	/**Make sure current lookahead symbol matches the given set
	 * Throw an exception upon mismatch, which is catch by either the
	 * error handler or by the syntactic predicate.
	 */
	void match(const BitSet& b);

	void matchNot(int t);

	static void panic();

	/** Parser error-reporting function can be overridden in subclass */
	virtual void reportError(const RecognitionException& ex);

	/** Parser error-reporting function can be overridden in subclass */
	virtual void reportError(const ANTLR_USE_NAMESPACE(std)string& s);

	/** Parser warning-reporting function can be overridden in subclass */
	virtual void reportWarning(const ANTLR_USE_NAMESPACE(std)string& s);

	virtual void rewind(int pos);

	/** Set the object used to generate ASTs */
//	void setASTFactory(ASTFactory astFactory_);

	/** Specify the type of node to create during tree building */
	void setASTNodeFactory(ASTFactory::factory_type factory);

	void setFilename(const ANTLR_USE_NAMESPACE(std)string& f);

	void setInputState(ParserSharedInputState state);

	/** Set or change the input token buffer */
//	void setTokenBuffer(TokenBuffer<Token>* t);

	virtual void traceIndent();
	virtual void traceIn(const ANTLR_USE_NAMESPACE(std)string& rname);
	virtual void traceOut(const ANTLR_USE_NAMESPACE(std)string& rname);
protected:
	int traceDepth;		// used to keep track of the indentation for the trace

protected:
	/** Utility class which allows tracing to work even when exceptions are
	 * thrown.
	 */
	class Tracer {
	private:
		Parser* parser;
		ANTLR_USE_NAMESPACE(std)string text;
	public:
		Tracer(Parser* p,const ANTLR_USE_NAMESPACE(std)string& t)
			: parser(p), text(t) { parser->traceIn(text); }
		~Tracer()
			{ parser->traceOut(text); }
	private:
		Tracer(const Tracer&);							// undefined
		const Tracer& operator=(const Tracer&);	// undefined
	};

private:
	Parser(const Parser&);								// undefined
	const Parser& operator=(const Parser&);		// undefined
};

ANTLR_END_NAMESPACE

#endif //INC_Parser_hpp__
