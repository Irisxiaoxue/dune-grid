// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <dune/grid/io/file/dgfparser/dgfprojectionblock.hh>

namespace Dune
{

  namespace dgf
  {

    // ProjectionBlock
    // ---------------

    const char *ProjectionBlock::ID = "Projection";

    ProjectionBlock::ProjectionBlock ( std::istream &in, int dimworld )
      : BasicBlock( in, ID ),
        defaultFunction_( 0 )
    {
      while( getnextline() )
      {
        //std::cout << "Projection line:";
        nextToken();

        if( token.type == Token::functionKeyword )
        {
          nextToken();
          parseFunction();
        }
        else if( token.type == Token::defaultKeyword )
        {
          nextToken();
          parseDefault();
        }
        else
          DUNE_THROW( DGFException, "Error in " << *this << ": Invalid token." );
      }
    }


    void ProjectionBlock::parseFunction ()
    {
      if( token.type != Token::string )
        DUNE_THROW( DGFException, "Error in " << *this << ": function name expected." );
      const std::string functionName = token.literal;
      if( functions_.find( functionName ) != functions_.end() )
        DUNE_THROW( DGFException, "Error in " << *this << ": redeclaration of function " << functionName << "." );
      nextToken();

      matchToken( Token::openingParen, "'(' expected." );
      if( token.type != Token::string )
        DUNE_THROW( DGFException, "Error in " << *this << ": variable name expected." );
      const std::string variableName = token.literal;
      nextToken();
      matchToken( Token::closingParen, "')' expected." );

      matchToken( Token::equals, "'=' expected." );
      const Expression *expression = parseExpression( variableName );

      matchToken( Token::endOfLine, "trailing tokens on line." );

      //std::cout << std::endl << "Declaring function: " << functionName << "( " << variableName << " )" << std::endl;
      functions_[ functionName ] = expression;
    }


    const ProjectionBlock::Expression *
    ProjectionBlock::parseUnaryExpression ( const std::string &variableName )
    {
      const Expression *expression;

      // parenthesized expression
      if( token.type == Token::openingParen )
      {
        nextToken();
        expression = parseExpression( variableName );
        matchToken( Token::closingParen, "')' expected." );
      }
      // norm expression
      else if( token.type == Token::normDelim )
      {
        nextToken();
        expression = new NormExpression( parseExpression( variableName ) );
        matchToken( Token::normDelim, "'|' expected." );
      }
      // number
      else if( token.type == Token::number )
      {
        expression = new NumberExpression( token.value );
        nextToken();
      }
      // unary minus
      else if( (token.type == Token::additiveOperator) && (token.symbol == '-') )
      {
        nextToken();
        expression = new MinusExpression( parseExpression( variableName ) );
      }
      // sqrt
      else if( token.type == Token::sqrtKeyword )
      {
        nextToken();
        matchToken( Token::openingParen, "'(' expected." );
        expression = new SqrtExpression( parseExpression( variableName ) );
        matchToken( Token::closingParen, "')' expected." );
      }
      else if( token.type == Token::string )
      {
        if( token.literal != variableName )
        {
          FunctionMap::iterator it = functions_.find( token.literal );
          if( it == functions_.end() )
            DUNE_THROW( DGFException, "Error in " << *this << ": function " << token.literal << " not declared." );
          nextToken();
          matchToken( Token::openingParen, "'(' expected." );
          expression = new FunctionCallExpression( it->second, parseExpression( variableName ) );
          matchToken( Token::closingParen, "')' expected." );
        }
        else
        {
          expression = new VariableExpression;
          nextToken();
        }
      }
      else
        DUNE_THROW( DGFException, "Error in " << *this << ": " << "unary expression expected." );

      return expression;
    }


    const ProjectionBlock::Expression *
    ProjectionBlock::parsePowerExpression ( const std::string &variableName )
    {
      const Expression *expression = parseUnaryExpression( variableName );
      while( token.type == Token::powerOperator )
      {
        nextToken();
        expression = new PowerExpression( expression, parseUnaryExpression( variableName ) );
      }
      return expression;
    }


    const ProjectionBlock::Expression *
    ProjectionBlock::parseMultiplicativeExpression ( const std::string &variableName )
    {
      const Expression *expression = parsePowerExpression( variableName );
      while( token.type == Token::multiplicativeOperator )
      {
        const char symbol = token.symbol;
        nextToken();
        if( symbol == '*' )
          expression = new ProductExpression( expression, parsePowerExpression( variableName ) );
        else if( symbol == '/' )
          expression = new QuotientExpression( expression, parsePowerExpression( variableName ) );
        else
          DUNE_THROW( DGFException, "Error in " << *this << ": Internal tokenizer error." );
      }
      return expression;
    }


    const ProjectionBlock::Expression *
    ProjectionBlock::parseExpression ( const std::string &variableName )
    {
      const Expression *expression = parseMultiplicativeExpression( variableName );
      while( token.type == Token::additiveOperator )
      {
        const char symbol = token.symbol;
        nextToken();
        if( symbol == '+' )
          expression = new SumExpression( expression, parseMultiplicativeExpression( variableName ) );
        else if( symbol == '-' )
          expression = new DifferenceExpression( expression, parseMultiplicativeExpression( variableName ) );
        else
          DUNE_THROW( DGFException, "Error in " << *this << ": Internal tokenizer error." );
      }
      return expression;
    }


    void ProjectionBlock::parseDefault ()
    {
      if( token.type != Token::string )
        DUNE_THROW( DGFException, "Error in " << *this << ": function name expected." );
      const std::string functionName = token.literal;
      nextToken();

      matchToken( Token::endOfLine, "trailing tokens on line." );

      //std::cout << std::endl << "Default function: " << functionName << std::endl;
      FunctionMap::iterator it = functions_.find( functionName );
      if( it == functions_.end() )
        DUNE_THROW( DGFException, "Error in " << *this << ": function " << functionName << " not declared." );
      defaultFunction_ = it->second;
    }


    void ProjectionBlock::matchToken ( const Token::Type &type, const std::string &message )
    {
      if( token.type != type )
        DUNE_THROW( DGFException, "Error in " << *this << ": " << message );
      if( type != Token::endOfLine )
        nextToken();
    }


    void ProjectionBlock::nextToken ()
    {
      int c;

      // eat white space
      while( ((c = line.peek()) == ' ') || (c == '\t') )
        line.get();

      // parse string literals
      if( ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) )
      {
        token.type = Token::string;
        token.literal = "";
        while( ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) )
        {
          token.literal += lowerCase( line.get() );
          c = line.peek();
        }

        if( token.literal == "default" )
          token.type = Token::defaultKeyword;
        if( token.literal == "function" )
          token.type = Token::functionKeyword;
        if( token.literal == "sqrt" )
          token.type = Token::sqrtKeyword;
      }
      // parse numeric constant
      else if( (c >= '0') && (c <= '9') )
      {
        token.type = Token::number;
        token.value = 0;
        while( (c >= '0') && (c <= '9') )
        {
          token.value = 10*token.value + double( c - '0' );
          token.literal += char( line.get() );
          c = line.peek();
        }
        if( c == '.' )
        {
          line.get();
          c = line.peek();
          double factor = 0.1;
          while( (c >= '0') && (c <= '9') )
          {
            token.value += factor * double( c - '0' );
            factor *= 0.1;
            c = line.peek();
          }
        }
      }
      // parse single character tokens
      else if( c == '=' )
        token.setSymbol( Token::equals, line.get() );
      else if( c == '(' )
        token.setSymbol( Token::openingParen, line.get() );
      else if( c == ')' )
        token.setSymbol( Token::closingParen, line.get() );
      else if( c == '|' )
        token.setSymbol( Token::normDelim, line.get() );
      else if( (c == '+') || (c == '-') )
        token.setSymbol( Token::additiveOperator, line.get() );
      else if( (c == '*') )
      {
        c = line.get();
        if( (line.peek() == '*') )
        {
          token.type = Token::powerOperator;
          line.get();
        }
        else
          token.setSymbol( Token::multiplicativeOperator, c );
      }
      else if( c == '/' )
        token.setSymbol( Token::multiplicativeOperator, line.get() );
      // parse end of line
      else if( c == EOF )
        token.type = Token::endOfLine;

      //std::cout << " " << token;
    }



    std::ostream &operator<< ( std::ostream &out, const ProjectionBlock::Token &token )
    {
      typedef ProjectionBlock::Token Token;
      switch( token.type )
      {
      case Token::string :
        return out << "string [" << token.literal << "]";
      case Token::number :
        return out << "number [" << token.value << "]";
      case Token::defaultKeyword :
        return out << "default";
      case Token::functionKeyword :
        return out << "function";
      case Token::sqrtKeyword :
        return out << "sqrt";
      case Token::equals :
        return out << "'='";
      case Token::openingParen :
        return out << "'('";
      case Token::closingParen :
        return out << "')'";
      case Token::normDelim :
        return out << "'|'";
      case Token::additiveOperator :
        return out << "addop [" << token.symbol << "]";
      case Token::multiplicativeOperator :
        return out << "mulop [" << token.symbol << "]";
      case Token::powerOperator :
        return out << "powerop" << std::endl;
      case Token::endOfLine :
        return out << "eol";
      default :
        return out << "invalid [" << token.type << "]";
      }
    }


    void ProjectionBlock::NumberExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      result.resize( 1 );
      result[ 0 ] = value_;
    }


    void ProjectionBlock::VariableExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      result = argument;
    }


    void ProjectionBlock::FunctionCallExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      Vector v;
      expression_->evaluate( argument, v );
      function_->evaluate( v, result );
    }


    void ProjectionBlock::MinusExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      expression_->evaluate( argument, result );
      const size_t size = result.size();
      for( size_t i = 0; i < size; ++i )
        result[ i ] *= -1.0;
    }


    void ProjectionBlock::NormExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      expression_->evaluate( argument, result );
      double normsqr = 0.0;
      const size_t size = result.size();
      for( size_t i = 0; i < size; ++i )
        normsqr += result[ i ] * result[ i ];
      result.resize( 1 );
      result[ 0 ] = sqrt( normsqr );
    }


    void ProjectionBlock::SqrtExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      expression_->evaluate( argument, result );
      if( result.size() != 1 )
        DUNE_THROW( MathError, "Cannot calculate square root of a vector." );
      result[ 0 ] = sqrt( result[ 0 ] );
    }


    void ProjectionBlock::PowerExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      exprA_->evaluate( argument, result );
      Vector v;
      exprB_->evaluate( argument, v );

      if( (result.size() == 1) && (v.size() == 1) )
        result[ 0 ] = std::pow( result[ 0 ], v[ 0 ] );
      else
        DUNE_THROW( MathError, "Cannot calculate powers of vectors." );
    }


    void ProjectionBlock::SumExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      exprA_->evaluate( argument, result );
      Vector v;
      exprB_->evaluate( argument, v );

      if( result.size() != v.size() )
      {
        const size_t size = result.size();
        for( size_t i = 0; i < size; ++i )
          result[ i ] += v[ i ];
      }
      else
        DUNE_THROW( MathError, "Cannot sum vectors of different size." );
    }


    void ProjectionBlock::DifferenceExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      exprA_->evaluate( argument, result );
      Vector v;
      exprB_->evaluate( argument, v );

      if( result.size() == v.size() )
      {
        const size_t size = result.size();
        for( size_t i = 0; i < size; ++i )
          result[ i ] -= v[ i ];
      }
      else
        DUNE_THROW( MathError, "Cannot sum vectors of different size." );
    }


    void ProjectionBlock::ProductExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      exprA_->evaluate( argument, result );
      Vector v;
      exprB_->evaluate( argument, v );

      if( result.size() == v.size() )
      {
        double product = 0.0;
        const size_t size = result.size();
        for( size_t i = 0; i < size; ++i )
          product += result[ i ] * v[ i ];
        result.resize( 1 );
        result[ 0 ] = product;
      }
      else if( v.size() == 1 )
      {
        const size_t size = result.size();
        for( size_t i = 0; i < size; ++i )
          result[ i ] *= v[ 0 ];
      }
      else if( result.size() == 1 )
      {
        std::swap( result, v );
        const size_t size = result.size();
        for( size_t i = 0; i < size; ++i )
          result[ i ] *= v[ 0 ];
      }
      else
        DUNE_THROW( MathError, "Cannot multiply non-scalar vectors of different size." );
    }


    void ProjectionBlock::QuotientExpression
    ::evaluate ( const Vector &argument, Vector &result ) const
    {
      exprA_->evaluate( argument, result );
      Vector v;
      exprB_->evaluate( argument, v );

      if( v.size() == 1 )
      {
        const size_t size = result.size();
        for( size_t i = 0; i < size; ++i )
          result[ i ] /= v[ 0 ];
      }
      else
        DUNE_THROW( MathError, "Cannot multiply non-scalar vectors of different size." );
    }

  }

}