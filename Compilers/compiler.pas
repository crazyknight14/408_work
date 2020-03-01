Program Compiler;
const
     norw = 11;         { no. of reserved words }
     txmax = 100;       { length of identifier table }
     nmax = 14;         { max. no of digits in numbers }
     al = 10;           { length of identifiers }
type
    symbol = (nul,ident,number,plus,minus,times,slash,oddsym,
     eql,neq,lss,leq,gtr,geq,lparen,rparen,comma,semicolon,colon,
     period,becomes,beginsym,endsym,ifsym,thensym,
     whilesym,dosym,callsym,constsym,varsym,procsym);
    alfa = packed array[1..al] of char;
    objects = (constant,variable,procdure);
var
   ch: char;            { last character read }
   sym:symbol;          { last symbol read }
   id :alfa;            { last identifier read }
   num,i: integer;      { last number read }
   charcnt : integer;   { character count }
   linlen : integer;    { line length }
   kk : integer;
   line: array [1..81] of char;
   a:alfa;
   word: array [1..norw] of alfa;
   wsym: array [1..norw] of symbol;
   ssym: array [char] of symbol;
   table: array [0..txmax] of record
            name: alfa;
            kind: objects
            end;

procedure error(I:integer);
begin
   case I of
     1:writeln(' Use =  instead of :=');
     2:writeln(' = must be followed by a number.');
     3:writeln(' Identifier must be followed by =.');
     4:writeln(' Const, Var, Procedure must be followed by an identifier.');
     5:writeln(' Semicolon or comma missing.');
     6:writeln(' Incorreet symbol after procedue declaration.');
     7:writeln(' Statement expected.');
     8:writeln(' Incorrect symbol after statement part in block.');
     9:writeln(' Period expected.');
     10:writeln(' Semicolon between statements is missing.');
     11:writeln(' Undeclared identifier.');
     12:writeln(' Assignment to constant  or  procedure is not allowed.');
     13:writeln(' Assignment operator := expected.');
     14:writeln(' call must be followed by an identifier.');
     15:writeln(' Call of a constant or a variable is meaningless.');
     16:writeln(' Then expected.');
     17:writeln(' Semicolon or end expected.');
     18:writeln(' DO expected.');
     19:writeln(' Incorrect symbol following statement.');
     20:writeln(' Relational operator expected.');
     21:writeln(' Expression must not contain a procedure identifier.');
     22:writeln(' Right parenthesis missing.');
     23:writeln(' The preceding factor cannot be followed by this symbol.');
     24:writeln(' An expression cannot begin with this symbol.');
     25:writeln(' This number is too large.');
   end;
   HALT
end;

procedure getsym; {this procedure classifies each token as a particular symbol. }
 var i,j,k:integer;

 procedure getch;
  {this procedure reads in a line at a time from the input file and stores
   it in an array. A character at atime is read from the array }
  begin {getch}
   if charcnt=linlen then
    begin
      if eof then
       begin
        writeln('PROGRAM INCOMPLETE');
        if (sym=endsym) then error(9);
       end;
      linlen:=0; charcnt:=0;
      while not eoln do
       begin
         linlen:=linlen+1;
         read(ch);
         write(ch);
         ch:=upcase(ch);
         line[linlen]:=ch {read in a line from input file }
       end;
      writeln();
      linlen:=linlen+1;
      readln();
      line[linlen]:=' ' { initialize last character or string as a spaces }
    end;
   charcnt:=charcnt+1;
   ch:=line[charcnt]
  end; {getch}

begin { getsym}
 while ch=' ' do getch;
 if ch in ['A'..'Z'] then
  begin { identifier or keyword}
   k:=0;
   repeat
    if k< al then
     begin
      k:=k+1;
      a[k]:=ch;
     end;
    getch
   until not (ch in ['A'..'Z','0'..'9']);
   if k >= kk then kk:=k
   else
    repeat
     a[kk]:=' ';
     kk:=kk-1
    until kk=k;
   id:=a;               {binary search to determine if id is a reseved word}
   i:=1;
   j:=norw;
   repeat
      k:=(i+j) div 2;
      if id <=word[k] then j:=k-1 ;
      if id >= word[k] then i:=k+1;
   until  i>j;
   if i-1>j then  sym:=wsym[k] { reserved word or identifier }
   else sym:=ident;
  end
 else
  if ch in ['0'..'9'] then
   begin
    k:=0;
    num:=0;
    sym:=number;
    repeat
     num:=10*num+(ord(ch)-ord('0'));
     k:=k+1;
     getch
    until not (ch in ['0'..'9']);
    if k>nmax then error(25)
   end
  else
   if ch=':' then
    begin
     getch;
     if ch='=' then
      begin
       sym:=becomes;
       getch
      end
     else
      sym:=colon
    end
    else
     if ch='>' then
      begin
       getch;
       if ch='=' then
        begin
         sym:=geq;
         getch
        end
       else sym:=gtr
      end
     else
      if ch='<' then
       begin
        getch;
        if ch='=' then
         begin
          sym:=leq;
          getch
         end
        else
         if ch='>' then
          begin
           sym:=neq;
           getch
          end
         else
          sym:=lss
       end
      else
       begin
        sym:=ssym[ch];
        getch
       end
end; {getsym}

procedure block(tx:integer);

function position(k:alfa):integer;
var
  i:integer;
begin
  table[0].name:=k;
  i:=tx;
  while table[i].name<>k do
   i:=i-1;
  position:=i;
end;

procedure enter(k:objects);
begin
         tx:=tx+1;
         with table[tx] do
          begin
            name:=id;
            kind:=k;
          end

end;

procedure constdeclaration;
begin
 if sym=ident then
  begin
   getsym;
   if sym=eql then
    begin
     getsym;
     if sym=number then
      begin
       enter(constant);
       getsym;
      end
     else error(2)
    end
   else error(3)
  end
 else error(4)
end;

procedure vardeclaration;
begin
 if sym=ident then
  begin
       enter(variable);
       getsym;
  end
 else error(4)
end;


procedure statement;

procedure expression;

 procedure term;

  procedure factor;
   var i : integer;

  begin { factor }
    if sym = ident then
     begin
       i := position(id);
       if i = 0  then error(11)
       else
       getsym
     end
    else
     if sym = number then
       getsym
      else
      if sym = lparen then
       begin
        getsym;
        expression;
        if sym = rparen then
         getsym
        else error(22)
       end
      else error(24)
  end; { factor }

 begin { term }
  factor;
  while sym in [times,slash] do
   begin
    getsym;
    factor;
   end;
 end; { term }

begin { expression }
 if sym in [plus,minus] then
  begin
   getsym;
   term;
  end
 else
  term;
  while sym in [plus,minus] do
   begin
    getsym;
    term;
   end;
end; { expression }

procedure condition;
 begin
    if sym = oddsym then
      begin
        getsym;
        expression;
      end
    else
     begin
        expression;
        if not (sym in [eql,neq,lss,leq,gtr,geq]) then
          error(20)
        else
         begin
           getsym;
           expression;
         end
     end
 end; { condition }

begin { statement }
 case sym of
 ident:begin
            i:=position(id);
            if i=0 then error(11)
            else
             if table[i].kind<>variable then error(12);
            getsym;
            if sym=becomes then getsym else error(13);
            expression;
         end;
 callsym:begin
            getsym;
            if sym<>ident then error(14)
            else
             begin
                  i:=position(id);
                  if i=0 then error(11)
                  else
                   if table[i].kind<>procdure then error(15);
                  getsym;
             end
         end;
 ifsym:begin
            getsym;
            condition;
            if sym=thensym then getsym else error(16);
            statement;
       end;
 beginsym:begin
            repeat
             getsym;
             statement
            until sym<>semicolon;
            if sym=endsym then getsym
            else error(17)
          end;
 whilesym:begin
            getsym;
            condition;
            if sym=dosym then getsym
            else error(18);
            statement;
          end;
end { case }
end; { statement }

begin { block }
     if sym=constsym then
      begin
         getsym;
         constdeclaration;
         while sym=comma do
          begin
            getsym;
            constdeclaration
          end;
         if sym=semicolon then getsym
         else error(10);
      end;
      if sym=varsym then
      begin
         getsym;
         vardeclaration;
         while sym=comma do
          begin
            getsym;
            vardeclaration
          end;
         if sym=semicolon then getsym
         else error(10);
      end;
      while sym=procsym do
       begin
           getsym;
           if sym=ident then
            begin
               enter(procdure);
               getsym
            end
           else error(4);
           if sym=semicolon then getsym
           else error(10);
           block(tx);
           if sym=semicolon then getsym
           else error(10);
       end;
    statement;
end; { block }

procedure Intialize;
begin
     for ch:=';' to 'A' do
       ssym[ch]:=nul;
     word[1]:='BEGIN     ';
     word[2]:='CALL      ';
     word[3]:='CONST     ';
     word[4]:='DO        ';
     word[5]:='END       ';
     word[6]:='IF        ';
     word[7]:='ODD       ';
     word[8]:='PROCEDURE ';
     word[9]:='THEN      ';
     word[10]:='VAR       ';
     word[11]:='WHILE     ';

     wsym[1]:=beginsym;
     wsym[2]:=callsym;
     wsym[3]:=constsym;
     wsym[4]:=dosym;
     wsym[5]:=endsym;
     wsym[6]:=ifsym;
     wsym[7]:=oddsym;
     wsym[8]:=procsym;
     wsym[9]:=thensym;
     wsym[10]:=varsym;
     wsym[11]:=whilesym;

     ssym['+']:=plus;
     ssym['-']:=minus;
     ssym['*']:=times;
     ssym['/']:=slash;
     ssym['(']:=lparen;
     ssym[')']:=rparen;
     ssym['=']:=eql;
     ssym[',']:=comma;
     ssym['.']:=period;
     ssym['#']:=neq;
     ssym['<']:=lss;
     ssym['>']:=gtr;
     ssym['"']:=leq;
     ssym['@']:=geq;
     ssym[';']:=semicolon;
     ssym[':']:=colon;

     charcnt:=0;
     linlen:=0;
     ch:=' ';
     kk:=al;
     a:='          ';
     id:='          ';
end;

BEGIN { main program }
      Intialize;
      getsym;
      block(0);
      if (sym<>period) then error(9);
      writeln;
      writeln('Successful compilation!');
      writeln;
End. { main program }
