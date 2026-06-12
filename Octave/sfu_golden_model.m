function [result_hex, result_dec] = sfu_golden_model(input_hex, func)
    add_golden_model_path();

    if nargin != 2
        error("sfu_golden_model requires input_hex and func");
    endif

    if isstring(input_hex)
        input_hex = char(input_hex);
    endif

    func = str2double(num2str(func));
    input = upper(input_hex);

    func_act = func;
    func_rro = func;

    [LUTC0,LUTC1,LUTC2,m] = loadLUTs(func);

    if (func == 6)
        input = char(hex2bin(input));

        s = input(1);
        exp = input(2:9);
        man = input(10:end);

        if (bin2dec(strcat(exp,".0"))>133)
            man = strcat('1','11111111','00000000000000000000000');

            vec = 1:32;
            for i=1:size(man,2)
                vec(i)=man(i)-48;
            endfor

            input = binaryVectorToHex(vec);
        else
            man = strcat("001",man,char(zeros(1,6)+48));
            man = strcat(char(zeros(1,133-bin2dec(strcat(exp,".0")))+48),man(1:end-(133-bin2dec(strcat(exp,".0")))));

            for i=1:size(man,2)
                if man(i)=='0' && s=='0'
                    man(i)='0';
                elseif man(i)=='0' && s=='1'
                    man(i)='1';
                elseif man(i)=='1' && s=='0'
                    man(i)='1';
                else
                    man(i)='0';
                endif
            endfor

            man = bin2dec(strcat(man,'.0'))+bin2dec(strcat(s,".0"));
            man = dec2bin(man,0);

            if (size(man,2))~=32
                man = strcat(char(zeros(1,32-size(man,2))+48),man);
            endif

            man = strcat('0',man(2:end));

            vec = 1:32;
            for i=1:size(man,2)
                vec(i)=man(i)-48;
            endfor

            input = binaryVectorToHex(vec);
        endif
    elseif (func == 9 || func == 10)
        input = hex754_2dec(input);

        if (input>0)
            s = '0';
        else
            input = input*-1;
            s = '1';
        endif

        if (input > pi()*2)
            vueltas = floor(input/(pi()*2));
            input = input-(pi()*2*vueltas);
        endif

        if (input>0 && input<=(pi()/2))
            Q='00';
        elseif(input>pi()/2 && input<=pi())
            Q='01';
        elseif(input>pi() && input<=((3*pi())/2))
            Q='10';
        else
            Q='11';
        endif

        if (Q=="01")
            input=input-(pi()/2);
        elseif (Q=="10")
            input=input-pi();
        elseif (Q=="11")
            input=input-((3*pi())/2);
        endif

        input=erase(dec2bin(input,23),'.');
        input=strcat(s,Q,'00000',input);

        vec = 1:32;
        for i=1:size(input,2)
            vec(i)=input(i)-48;
        endfor

        input = binaryVectorToHex(vec);
    endif

    if func == 2
        aux = char(hex2bin(input));
        if aux(9) == '0' && func_act == 2
            [LUTC0,LUTC1,LUTC2,m] = loadLUTs(3);
            func_act = 3;
        elseif aux(9) == '1' && func_act == 3
            [LUTC0,LUTC1,LUTC2,m] = loadLUTs(2);
            func_act = 2;
        endif
    elseif func == 4
        aux = char(hex2bin(input));
        if aux(9) == '0' && func_act == 4
            [LUTC0,LUTC1,LUTC2,m] = loadLUTs(5);
            func_act = 5;
        elseif aux(9) == '1' && func_act == 5
            [LUTC0,LUTC1,LUTC2,m] = loadLUTs(4);
            func_act = 4;
        endif
    elseif func == 7
        aux = char(hex2bin(input));
        aux = bin2dec(strcat(aux(2:9),'.0'))-127;
        if aux == 0 && func_act == 7
            [LUTC0,LUTC1,LUTC2,m] = loadLUTs(8);
            func_act = 8;
        elseif aux ~= 0 && func_act == 8
            [LUTC0,LUTC1,LUTC2,m] = loadLUTs(7);
            func_act = 7;
        endif
    elseif func == 9
        aux = char(hex2bin(input));
        if  (aux(1) == '0' && aux(2) == '0' && aux(3) == '1') ||...
            (aux(1) == '0' && aux(2) == '1' && aux(3) == '1') ||...
            (aux(1) == '1' && aux(2) == '0' && aux(3) == '1') ||...
            (aux(1) == '1' && aux(2) == '1' && aux(3) == '1')
            func_rro = 10;
        else
            func_rro = 9;
        endif

        if func_rro == 9
            aux = bin2dec(strcat(aux(9),'.',aux(10:end)));
            if aux>=1 && func_act == 9
                [LUTC0,LUTC1,LUTC2,m] = loadLUTs(10);
                func_act = 10;
            elseif aux<1 && func_act == 10
                [LUTC0,LUTC1,LUTC2,m] = loadLUTs(9);
                func_act = 9;
            endif
        elseif func_rro == 10
            aux = bin2dec(strcat(aux(9),'.',aux(10:end)));
            if aux>=1 && func_act == 10
                [LUTC0,LUTC1,LUTC2,m] = loadLUTs(9);
                func_act = 9;
            elseif aux<1 && func_act == 9
                [LUTC0,LUTC1,LUTC2,m] = loadLUTs(10);
                func_act = 10;
            endif
        endif
    elseif func == 10
        aux = char(hex2bin(input));
        if  (aux(1) == '0' && aux(2) == '0' && aux(3) == '1') ||...
            (aux(1) == '0' && aux(2) == '1' && aux(3) == '1') ||...
            (aux(1) == '1' && aux(2) == '0' && aux(3) == '1') ||...
            (aux(1) == '1' && aux(2) == '1' && aux(3) == '1')
            func_rro = 9;
        else
            func_rro = 10;
        endif

        if func_rro == 10
            aux = bin2dec(strcat(aux(9),'.',aux(10:end)));
            if aux>=1 && func_act == 10
                [LUTC0,LUTC1,LUTC2,m] = loadLUTs(9);
                func_act = 9;
            elseif aux<1 && func_act == 9
                [LUTC0,LUTC1,LUTC2,m] = loadLUTs(10);
                func_act = 10;
            endif
        elseif func_rro == 9
            aux = bin2dec(strcat(aux(9),'.',aux(10:end)));
            if aux>=1 && func_act == 9
                [LUTC0,LUTC1,LUTC2,m] = loadLUTs(10);
                func_act = 10;
            elseif aux<1 && func_act == 10
                [LUTC0,LUTC1,LUTC2,m] = loadLUTs(9);
                func_act = 9;
            endif
        endif
    endif

    if func == 6
        aux = char(hex2bin(input));
        man = aux(10:end);
    elseif func == 9
        aux = char(hex2bin(input));
        if  (aux(1) == '0' && aux(2) == '1' && aux(3) == '0') ||...
            (aux(1) == '0' && aux(2) == '1' && aux(3) == '1') ||...
            (aux(1) == '1' && aux(2) == '0' && aux(3) == '0') ||...
            (aux(1) == '1' && aux(2) == '0' && aux(3) == '1')
            s = '1';
        else
            s = '0';
        endif

        aux = bin2dec(strcat(aux(9),'.',aux(10:end)));
        if aux>=1
            Pi_2 = bin2dec('110010010000111111011011.0');
            man = char(hex2bin(input));
            man = man(9:end);
            man = bin2dec(strcat(man,'.0'));
            man = Pi_2-man;
            man = dec2bin(man,0);
            man = strcat(char(zeros(1,23-size(man,2))+48),man);
        else
            man = char(hex2bin(input));
            man = man(10:end);
        endif
    elseif func == 10
        aux = char(hex2bin(input));
        if  (aux(1) == '0' && aux(2) == '0' && aux(3) == '1') ||...
            (aux(1) == '0' && aux(2) == '1' && aux(3) == '0') ||...
            (aux(1) == '1' && aux(2) == '0' && aux(3) == '1') ||...
            (aux(1) == '1' && aux(2) == '1' && aux(3) == '0')
            s = '1';
        else
            s = '0';
        endif

        aux = bin2dec(strcat(aux(9),'.',aux(10:end)));
        if aux>=1
            Pi_2 = bin2dec('110010010000111111011011.0');
            man = char(hex2bin(input));
            man = man(9:end);
            man = bin2dec(strcat(man,'.0'));
            man = Pi_2-man;
            man = dec2bin(man,0);
            man = strcat(char(zeros(1,23-size(man,2))+48),man);
        else
            man = char(hex2bin(input));
            man = man(10:end);
        endif
    else
        number = char(hex2bin(input));
        s = number(1);
        exp = number(2:9);
        man = number(10:end);
    endif

    x1 = bin2dec(strcat(man(1:m),'.0'))+1;
    x2 = floor(bin2dec(strcat('0.',char(zeros(1,m)+48),man(m+1:end)))*2^23);

    C0 = LUTC0(x1,:);
    C1 = LUTC1(x1,:);
    C2 = LUTC2(x1,:);

    sign = [1 -1];
    operation = bin2dec(strcat(C0(2:end),'.0'))*2^14*sign(str2double(C0(1))+1)+...
                bin2dec(strcat(C1(2:end),'.0'))*x2*sign(str2double(C1(1))+1)+...
                (bin2dec(strcat(C2(2:end),'.0'))*floor(((bin2dec(strcat(man(m+1:end),'.0'))^2)*2^-19)))*2^1*sign(str2double(C2(1))+1);

    if func == 1
        result_hex = dec2hex754((operation*2^-41)*(2^-(bin2dec(strcat(exp,'.0'))-127))*sign(str2double(s)+1));
    elseif func == 2
        if func_act == 2
            result_hex = dec2hex754((operation*2^-41)*(2^((bin2dec(strcat(exp,'.0'))-127)/2)));
        else
            result_hex = dec2hex754((operation*2^-41)*(2^((bin2dec(strcat(exp,'.0'))-127-1)/2)));
        endif
    elseif func == 4
        if func_act == 4
            result_hex = dec2hex754((operation*2^-41)*(2^-((bin2dec(strcat(exp,'.0'))-127)/2)));
        else
            result_hex = dec2hex754((operation*2^-41)*(2^-((bin2dec(strcat(exp,'.0'))-127-1)/2)));
        endif
    elseif func == 6
        aux = char(hex2bin(input));
        if aux(2) == '1'
            aux = aux(2:9);
            for i=1:size(aux,2)
                if aux(i) == '0'
                    aux(i) = '1';
                else
                    aux(i) = '0';
                endif
            endfor
            exp = (bin2dec(strcat(aux,'.0'))+1)*-1;
        else
            exp = bin2dec(strcat(aux(2:9),'.0'));
        endif
        result_hex = dec2hex754((operation*2^-41)*(2^(exp)));
    elseif func == 7
        if func_act == 8
            result_hex = dec2hex754((operation*2^-41)*(hex754_2dec(input)-1));
        else
            result_hex = dec2hex754((operation*2^-41)+(bin2dec(strcat(exp,'.0'))-127));
        endif
    elseif func == 9 || func == 10
        result_hex = dec2hex754(operation*2^-41*sign(str2double(s)+1));
    else
        error("Unsupported function selector: %d", func);
    endif

    if func == 6
        input = char(hex2bin(input));

        s = input(1);
        exp = input(2:9);
        man = input(10:end);

        if (s=='1' && exp=="00000000")
            result_hex=binaryVectorToHex([0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]);
        elseif (s=='1' && exp=="11111111")
            result_hex=strcat('0',exp,man);

            vec = 1:32;
            for i=1:size(result_hex,2)
                vec(i)=result_hex(i)-48;
            endfor

            result_hex = binaryVectorToHex(vec);
        endif
    endif

    result_hex = char(upper(result_hex));
    result_dec = hex754_2dec(result_hex);
endfunction
