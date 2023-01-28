----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    09:20:52 04/01/2013 
-- Design Name: 
-- Module Name:    pwm_check - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity pwm_check is
    Port ( 
			clk_1060n : in STD_LOGIC;
			pwm_check_up : in STD_LOGIC;
			pwm_check_down : in STD_LOGIC;
			pwm_ok : out  STD_LOGIC);
end pwm_check;

architecture Behavioral of pwm_check is

constant max_pwm : STD_LOGIC_VECTOR(7 downto 0) := x"64"; -- x"46" 70*1.06us =74.2us ;; x"64" 100*1.06us =106us
begin


check_pwm:process(clk_1060n)
variable counter_pwm: STD_LOGIC_VECTOR(7 downto 0);
variable past_pwm : STD_LOGIC;
begin
	if rising_edge(clk_1060n) then
		if (pwm_check_up or pwm_check_down) = '0' and past_pwm='1' then
		
			counter_pwm:= x"00";
		else 
			
			if counter_pwm > max_pwm then
				pwm_ok <= '0';
			else
				pwm_ok <= '1';
				counter_pwm:=counter_pwm + 1;
			end if;
		end if;
		past_pwm := (pwm_check_up or pwm_check_down) ;
	end if;

end process;	--check_pwm

end Behavioral;

