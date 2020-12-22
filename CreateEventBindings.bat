<span style="font-size:14px;">@ECHO OFF
TITLE WmiDeleteEvent
COLOR A
echo -----------WmiDeleteEvent-----------
wmic /NAMESPACE:"\\root\subscription" PATH __FilterToConsumerBinding CREATE Filter="__EventFilter.Name=\"BotFilter82\"",Consumer=\"CommandLineEventConsumer.Name=\"BotConsumer23\""
echo -----------------------------------
IPCONFIG
echo=
echo -----------------------------------
ARP?
echo=
echo -----------------------------------
PAUSE</span>
