
-- written by Robert Sanchez

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity gigatron is
  Port ( 
         sysclk : in std_logic;
         VGA_R : out std_logic_vector(3 downto 0);
         VGA_G : out std_logic_vector(3 downto 0);
         VGA_B : out std_logic_vector(3 downto 0);
         VGA_HS : out std_logic; 
         VGA_VS : out std_logic;
         
         LED : out std_logic_vector(15 downto 0));
end gigatron;


--        TRUTH TABLE for A and B ALU operands (BA bit order for input)
-- LD  AR 0011
-- XOR AR 0110
-- OR  AR 0111
-- AND AR 0001
-- SUB AR 1100 (using AL too, NOTE AR0 here also carry-in)
--        0101 <-second side of mux
-- ADD AR 0011 (using AL too)
--        0101 <-second side of mux


architecture Behavioral of gigatron is

component blk_mem_gen_0
  PORT (
    clka : IN STD_LOGIC;
    ena : IN STD_LOGIC;
    wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    addra : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    dina : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    douta : OUT STD_LOGIC_VECTOR(15 DOWNTO 0)
  );
END component;

component blk_mem_gen_1
  PORT (
    clka : IN STD_LOGIC;
    ena : IN STD_LOGIC;
    wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    addra : IN STD_LOGIC_VECTOR(14 DOWNTO 0);
    dina : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    douta : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
  );
END component;

component clk_wiz_0
    port(
        clk_out1 : out std_logic;
        clk_out2 : out std_logic;
        clk_in1 : in std_logic
    );
end component;


    signal PC : unsigned(15 downto 0) := X"0000";
    signal DR : std_logic_vector(7 downto 0) := X"00";
    signal IR : std_logic_vector(7 downto 0) := X"00";
    
    signal AC    : std_logic_vector(7 downto 0) := X"00";
    signal Y     : std_logic_vector(7 downto 0) := X"00";
    signal X     : std_logic_vector(7 downto 0) := X"00";
    signal OUTR  : std_logic_vector(7 downto 0) := X"00";
    signal INR   : std_logic_vector(7 downto 0) := X"FF";
    signal XOUTR : std_logic_vector(7 downto 0) := X"00";
    
    signal DIN : std_logic_vector(15 downto 0);
    signal DOUT : std_logic_vector(15 downto 0);
    
    signal RAMDIN : std_logic_vector(7 downto 0);
    signal RAMDOUT : std_logic_vector(7 downto 0);
    

    signal DBUS  : std_logic_vector(7 downto 0) := X"00";
    
    signal TMP : std_logic_vector(7 downto 0) := X"00";
    signal MAR : std_logic_vector(15 downto 0) := "0000000000000000";
    
    signal RAMWE : std_logic_vector(0 downto 0) := "0";
    
    signal PCWE : std_logic := '0';
    signal PCNEW : unsigned(15 downto 0) := X"0000";
    
    signal fastclk : std_logic;
    signal clk : std_logic;
    

begin

    ROM : blk_mem_gen_0 port map(clk, '1', "0", std_logic_vector(PC), DIN, DOUT);
    RAM : blk_mem_gen_1 port map(fastclk, '1', RAMWE, MAR(14 downto 0), RAMDIN, RAMDOUT);
    CLOCK : clk_wiz_0 port map(clk, fastclk, sysclk);
    -- memory address process
    process(DOUT, DR, X, Y)
    begin
            if IR(7 downto 5) = "111" then
                MAR <= "00000000" & DR;
            else
            case (DOUT(4 downto 2)) is
                when "000" =>
                    MAR <= "00000000" & DR;
                when "001" =>
                    MAR <= "00000000" & X;
                when "010" =>
                    MAR <= Y & DR;
                when "011" =>
                    MAR <= Y & X;
                when "100" =>
                    MAR <= "00000000" & DR;
                when "101" =>
                    MAR <= "00000000" & DR;
                when "110" =>
                    MAR <= "00000000" & DR;
                when "111" =>
                    MAR <= Y & X;
                when others =>
            end case;
            end if;
    end process;
    
    
    -- data bus value process
    process(DOUT, RAMDOUT, AC, INR)
    begin
            case (DOUT(1 downto 0)) is
                when "00" =>
                    DBUS <= DOUT(15 downto 8);
                when "01" =>
                    DBUS <= RAMDOUT; 
                when "10" => 
                    DBUS <= AC;
                when "11" => 
                    DBUS <= INR;
                when others =>
            end case;
    end process;


    
    -- real assignment process (write databus tmp value to destination)
    process(clk)
    begin
        RAMDIN <= TMP;

        if rising_edge(clk) then
        if (IR(7 downto 5) /= "111") then
        case (IR(4 downto 2)) is
        when "000" =>
            if IR(7 downto 5) /= "110" then
            AC <= TMP;
            end if;
        when "001" =>
            if IR(7 downto 5) /= "110" then
            AC <= TMP;
            end if;
        when "010" =>
            if IR(7 downto 5) /= "110" then
            AC <= TMP;
            end if;
        when "011" =>
            if IR(7 downto 5) /= "110" then
            AC <= TMP;
            end if;
        when "100" => --x
            X <= TMP;
        when "101" => --y
            Y <= TMP;
        when "110" => --out
            if IR(7 downto 5) /= "110" then
            OUTR <= TMP;
            end if;
        when "111" => --out
            if IR(7 downto 5) /= "110" then
            OUTR <= TMP;
            end if;
            X <= std_logic_vector(unsigned(X) + 1);
        when others =>
            if IR(7 downto 5) /= "110" then
            AC <= TMP;
            end if;
        end case;
        end if;
        end if;
    end process;

    -- program counter
    process(clk) 
    begin
        if rising_edge(clk) then
            if (PCWE = '1') then
                PC <= PCNEW;
            else
                PC <= PC + 1;
            end if;
        end if;
    end process;
    
    process(DOUT)
    begin
            RAMWE <= "0";
            if (DOUT(7 downto 5) = "110") then -- STORE
                RAMWE <= "1";
            end if;
    end process;

    -- select which value to write to data bus 
    process(DBUS,IR) 
    begin
            PCWE <= '0';
            
            case (IR(7 downto 5)) is
                when "000" =>   -- LD
                    TMP <= DBUS;
                when "001" =>   -- AND
                    TMP <= AC and DBUS;
                when "010" =>   -- OR
                    TMP <= AC or DBUS;
                when "011" =>   -- XOR
                    TMP <= AC xor DBUS;
                when "100" =>   -- ADD
                    TMP <= std_logic_vector(unsigned(AC) + unsigned(DBUS));
                when "101" =>   -- SUB
                    TMP <= std_logic_vector(unsigned(AC) - unsigned(DBUS));
                when "110" =>   -- STORE
                    TMP <= DBUS; --AC; 
                when "111" =>   -- JMP
                    case (IR(4 downto 2)) is
                        when "000" => -- far jump y, bus
                            PCNEW <= unsigned(Y & DBUS);
                            PCWE <= '1';
                        when "001" =>
                            if (signed(AC) > 0) then
                                PCNEW <= PC(15 downto 8) & unsigned(DBUS);
                                PCWE <= '1';
                            end if;
                        when "010" =>
                            if (signed(AC) < 0) then
                                PCNEW <= PC(15 downto 8) & unsigned(DBUS);
                                PCWE <= '1';
                            end if;
                        when "011" =>
                            if (signed(AC) /= 0) then
                                PCNEW <= PC(15 downto 8) & unsigned(DBUS);
                                PCWE <= '1';
                            end if;
                        when "100" =>
                            if (signed(AC) = 0) then
                                PCNEW <= PC(15 downto 8) & unsigned(DBUS);
                                PCWE <= '1';
                            end if;
                        when "101" =>
                            if (signed(AC) >= 0) then
                                PCNEW <= PC(15 downto 8) & unsigned(DBUS);
                                PCWE <= '1';
                            end if;
                        when "110" =>
                            if (signed(AC) <= 0) then
                                PCNEW <= PC(15 downto 8) & unsigned(DBUS);
                                PCWE <= '1';
                            end if;
                        when "111" =>
                            PCNEW <= PC(15 downto 8) & unsigned(DBUS);
                            PCWE <= '1';
                    when others =>
                    end case;
                when others =>
            end case;
    end process;
    
    process(OUTR(6))
    begin
        if rising_edge(OUTR(6)) then
            XOUTR <= AC;
        end if;
    end process;
    
    DR <= DOUT(15 downto 8);
    IR <= DOUT(7 downto 0);
    VGA_R <= OUTR(1 downto 0) & "00";
    VGA_G <= OUTR(3 downto 2) & "00";
    VGA_B <= OUTR(5 downto 4) & "00";
    VGA_HS <= OUTR(6);
    VGA_VS <= OUTR(7);
    LED(7 downto 0) <= XOUTR(7 downto 0);
    
end Behavioral;
