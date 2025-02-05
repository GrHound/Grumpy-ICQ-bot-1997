/* Most of the transliation by Lalo Martins <lalo@webcom.com> */

/* Help categories*/
#define CLIENT_CAT_STR "Cliente"
#define MESSAGE_CAT_STR "Mensagem"
#define USER_CAT_STR "Usu�rio"
#define ACCOUNT_CAT_STR "Conta"

/* Help strings for each command */
#define PASS_HELP_STR "\t\tMuda sua senha para \"secret\".\n"
#define MSG_HELP_STR "\t\tEnvia mensagem ao uin especificado.\n"
#define URL_HELP_STR "\tEnvia uma mensagem e URL ao uin especificado.\n"
#define MSGA_HELP_STR "\tEnvia uma mensagem de m�ltiplas linhas a todos em sua lista.\n"
#define AGAIN_HELP_STR "\t\tEnvia uma mensagem � �ltima pessoa a quem voc� enviou mensagem.\n"
#define AUTH_HELP_STR "\t\tAutoriza o usu�rio do uin a acrescentar voc� a sua lista.\n"
#define REPLY_HELP_STR "\t\tEnvia mensagem � �ltima pessoa que enviou a voc� (responde).\n"
#define RAND_HELP_STR "\t\tEncontra um usu�rio aleat�rio no grupo ou lista de grupos.\n"
#define LIST_HELP_STR "\t\t\tMostra o estado atual de todos em sua listaa de contatos.\n"
#define ALTER_HELP_STR "\tEste comando permite alterar durante execu��o o conjunto de comandos.\n"
#define VERBOSE_HELP_STR "\t\t\tAltera o n�vel de mensagens adicionais ( normalmente 0 ).\n"
#define CLEAR_HELP_STR "\t\t\t\tLimpa a tela.\n"
#define SOUND_HELP_STR "\t\t\t\tLiga/desliga o ru�do ao receber mensagens.\n"
#define COLOR_HELP_STR "\t\t\t\tLiga/desliga as cores.\n"
#define QUIT_HELP_STR "\t\t\tDesconecta e encerra o programa.\n"
#define AUTO1_HELP_STR "\t\t\t\tMostra seu estado atual.\n"
#define AUTO2_HELP_STR "\t\t\tLiga/desliga a habilidade de enviar mensagens quando DND, NA, etc.\n"
#define AUTO3_HELP_STR "\t\tConfigura a resposta autom�tica a ser usada quando nesse estado.\n"
#define REG_HELP_STR "\tCria um novo UIN com a senha especificada.\n"
#define CHANGE_HELP_STR "\tMuda seu estado para o n�mero especificado.\n\t\tSem o n�mero, lista os estados dispon�veis.\n"
#define ADD_HELP_STR "\tAcrescenta o UIN especificado � sua lista sob o nick \"nickname\".\n"
#define SEARCH_HELP_STR "\tBusca um usu�rio ICQ com o endere�o de e-mail especificado.\n"
#define INFO_HELP_STR "\t\tExibe informa��es sobre o UIN especificado.\n"
#define ONLINE_HELP_STR "\t\tMuda seu estado para Online (conectado).\n"
#define AWAY_HELP_STR "\t\tMuda seu estado para Away (fora).\n"
#define NA_HELP_STR "\t\tMuda seu estado para Not Available (n�o dispon�vel).\n"
#define OCC_HELP_STR "\t\tMuda seu estado para Occupied (ocupado).\n"
#define DND_HELP_STR "\t\tMuda seu estado para Do not Disturb (n�o perturbe).\n"
#define FFC_HELP_STR "\t\tMuda seu estado para Free for Chat (dispon�vel para chat).\n"
#define INV_HELP_STR "\t\tMuda seu estado para Invisible (invis�vel).\n"
#define UPDATE_HELP_STR "\t\tAtualiza sua informa��o b�sica (email, nick, etc.)\n"
#define SET_RAND_HELP_STR "\t\tConfigura seu grupo de usu�rios aleat�rios.\n"

#define VERBOSE_LEVEL_STR "N�vel de mensagens adicionais � %d.\n"
#define NICK_NOT_FOUND_STR "%s n�o � reconhecido como um nickname."

/* Used in the "w" user display */
#define W_ONLINE_SINCE_STR " Conectado desde %s"
#define W_LAST_ONLINE_STR " Conectado pela �ltima vez em %s"
#define W_LAST_ON_UNKNOWN_STR " �ltima conex�o: desconhecido."
#define W_STATUS_STR "Seu estado � "
#define W_OFFLINE_STR "Usu�rios desconectados: "
#define W_ONLINE_STR "Usu�rios conectados: "

#define MSG_PROMPT_STR "msg> "
#define MSGA_PROMPT_STR "msg todos> "

/* These are used durring user info updates. */
#define NICK_NAME_UPDATE_STR "Escolha seu novo Nick : "
#define FIRST_NAME_UPDATE_STR "Escolha seu novo Primeiro nome : "
#define LAST_NAME_UPDATE_STR "Escolha seu novo Sobrenome : "
#define EMAIL_UPDATE_STR "Escolha seu novo Endere�o de email : "
#define AUTH_QUESTION_STR "Voc� quer que usu�rios Mirabilis tenham que pedir sua autoriza��o? : "

#define YESNO_RESPONSE_STR "Por favor escolha " YES_STR " ou " NO_STR "\n"
#define MESSAGE_BUFFER_FULL_STR "Mensagem enviada antes que o espa�o se esgotasse\n"
#define MESSAGE_CANCELED_STR "Mensagem cancelada\n"
/* Having 2 strings with the nickname inserted in the middle */
/* will hopefully solve any potential word order problems */
#define MESSAGE_SENT_1_STR  "Mensagem enviada para "
#define MESSAGE_SENT_2_STR  "!\n"

/********************************************************************/

/* Simple Yes no response*/
#define YES_STR "SIM"
#define NO_STR "NAO"

/* The promt used by Micq */
#define PROMPT_STR SERVCOL "Micq> " NOCOL

/* The different status modes */
#define STATUS_ONLINE_STR "Online (conectado)"
#define STATUS_DND_STR "Away (fora)"
#define STATUS_AWAY_STR "Not Available (n�o dispon�vel)"
#define STATUS_NA_STR "Occupied (ocupado)"
#define STATUS_OCCUPIED_STR "Do not Disturb (n�o perturbe)"
#define STATUS_INVISIBLE_STR "Free for Chat (dispon�vel para chat)"
#define STATUS_FFC_STR "Invisible (invis�vel)"

/* results of searches */
#define ALL_FOUND_STR "Todos os usu�rois poss�veis foram encontrados."
#define TOO_MANY_STR "Usuarios demais foram encontrados."
#define SEARCH_DONE_STR "Pesquisa completa."

/* Strings for help screen */
#define SELECT_GROUP_STR "Escolha um dos t�picos de ajuda abaixo."
#define CLIENT_HELP_STR "Comandos sobre o mICQ e suas configura��es."
#define USER_HELP_STR "Comandos para achar outros usu�rios."
#define ACCOUNT_HELP_STR "Comandos sobre sua conta de ICQ."
#define MESSAGE_HELP_STR "Comandos sobre o envio de mensagens."

/* Various Strings */
#define LENGTH_STR "Tamanho"
#define EXTRA_DATA_STR "Dados extas"
#define SEQ_STR "SEQ"
#define VER_STR "Ver"
#define LOGIN_SUCCESS_STR "Conectado ao servidor ICQ com sucesso!"
#define SERVER_ACK_STR "O servidor recebeu o comando %04x."

/* The Countries there's alot */
#define USA_COUNTRY_STR "USA"
#define Afghanistan_COUNTRY_STR (const char*) "Afeganist�o"
#define Albania_COUNTRY_STR "Alb�nia"
#define Algeria_COUNTRY_STR "Alg�ria"
#define American_Samoa_COUNTRY_STR "Samoa Americana"
#define Andorra_COUNTRY_STR "Andorra"
#define Angola_COUNTRY_STR "Angola"
#define Anguilla_COUNTRY_STR "Anguilla"
#define Antigua_COUNTRY_STR "Ant�gua"
#define Argentina_COUNTRY_STR "Argentina"
#define Armenia_COUNTRY_STR "Arm�nia"
#define Aruba_COUNTRY_STR "Aruba"
#define Ascention_Island_COUNTRY_STR "Ilha da Ascen��o"
#define Australia_COUNTRY_STR "Austr�lia"
#define Australian_Antartic_Territory_COUNTRY_STR "Territ�rio ant�rtico Australiano"
#define Austria_COUNTRY_STR "�ustria"
#define Azerbaijan_COUNTRY_STR "Azerbaj�o"
#define Bahamas_COUNTRY_STR "Bahamas"
#define Bahrain_COUNTRY_STR "Bahrain"
#define Bangladesh_COUNTRY_STR "Bangladesh"
#define Barbados_COUNTRY_STR "Barbados"
#define Belarus_COUNTRY_STR "Belarus"
#define Belgium_COUNTRY_STR "B�lgica"
#define Belize_COUNTRY_STR "Belize"
#define Benin_COUNTRY_STR "Benin"
#define Bermuda_COUNTRY_STR "Bermudas"
#define Bhutan_COUNTRY_STR "But�o"
#define Bolivia_COUNTRY_STR "Bol�via"
#define Bosnia_Herzegovina_COUNTRY_STR "B�snia & Herzegovina"
#define Botswana_COUNTRY_STR "Botswana"
#define Brazil_COUNTRY_STR "Brasil"
#define British_Virgin_Islands_COUNTRY_STR "Ilhas Virgens Brit�nicas"
#define Brunei_COUNTRY_STR "Brunei"
#define Bulgaria_COUNTRY_STR "Bulg�ria"
#define Burkina_Faso_COUNTRY_STR "Burkina Faso"
#define Burundi_COUNTRY_STR "Burundi"
#define Cambodia_COUNTRY_STR "Camb�ja"
#define Cameroon_COUNTRY_STR "Camar�es"
#define Canada_COUNTRY_STR "Canad�"
#define Cape_Verde_Islands_COUNTRY_STR "Cabo Verde"
#define Cayman_Islands_COUNTRY_STR "Ilhas Cayman"
#define Central_African_Republic_COUNTRY_STR "Rep�blica Centro-Africana"
#define Chad_COUNTRY_STR "Chad"
#define Christmas_Island_COUNTRY_STR "Christmas Island"
#define Cocos_Keeling_Islands_COUNTRY_STR "Cocos-Keeling Islands"
#define Comoros_COUNTRY_STR "Comoros"
#define Congo_COUNTRY_STR "Congo"
#define Cook_Islands_COUNTRY_STR "Cook Islands"
#define Chile_COUNTRY_STR "Chile"
#define China_COUNTRY_STR "China"
#define Columbia_COUNTRY_STR "Col�mbia"
#define Costa_Rice_COUNTRY_STR "Costa Rice"
#define Croatia_COUNTRY_STR "Cro�cia"
#define Cuba_COUNTRY_STR "Cuba"
#define Cyprus_COUNTRY_STR "Cyprus"
#define Czech_Republic_COUNTRY_STR "Rep�blica Checa"
#define Denmark_COUNTRY_STR "Dinamarca"
#define Diego_Garcia_COUNTRY_STR "Diego Garcia"
#define Djibouti_COUNTRY_STR "Djibouti"
#define Dominica_COUNTRY_STR "Dominica"
#define Dominican_Republic_COUNTRY_STR "Republica Dominicana"
#define Ecuador_COUNTRY_STR "Equador"
#define Egypt_COUNTRY_STR "Egito"
#define El_Salvador_COUNTRY_STR "El Salvador"
#define Equitorial_Guinea_COUNTRY_STR "Guin� Equatorial"
#define Eritrea_COUNTRY_STR "Eritrea"
#define Estonia_COUNTRY_STR "Est�nia"
#define Ethiopia_COUNTRY_STR "Eti�pia"
#define Former_Yugoslavia_COUNTRY_STR "F.Y.R.O.M. (Antiga Yugoslavia)"
#define Faeroe_Islands_COUNTRY_STR "Faeroe Islands"
#define Falkland_Islands_COUNTRY_STR "Ilhas Falkland (Malvinas)"
#define Federated_States_of_Micronesia_COUNTRY_STR "Estados Federados da Micron�sia"
#define Fiji_COUNTRY_STR "Fiji"
#define Finland_COUNTRY_STR "Finl�ndia"
#define France_COUNTRY_STR "Fran�a"
#define French_Antilles_COUNTRY_STR "Antilhas Francesas"
#define French_Guiana_COUNTRY_STR "Guiana Francesa"
#define French_Polynesia_COUNTRY_STR "Polin�sia Francesa"
#define Gabon_COUNTRY_STR "Gab�o"
#define Gambia_COUNTRY_STR "G�mbia"
#define Georgia_COUNTRY_STR "Ge�rgia"
#define Germany_COUNTRY_STR "Alemanha"
#define Ghana_COUNTRY_STR "Ghana"
#define Gibraltar_COUNTRY_STR "Gibraltar"
#define Greece_COUNTRY_STR "Gr�cia"
#define Greenland_COUNTRY_STR "Groenl�ndia"
#define Grenada_COUNTRY_STR "Granada"
#define Guadeloupe_COUNTRY_STR "Guadalupe"
#define Guam_COUNTRY_STR "Guam"
#define Guantanomo_Bay_COUNTRY_STR "Ba�a de Guantanomo"
#define Guatemala_COUNTRY_STR "Guatemala"
#define Guinea_COUNTRY_STR "Guin�"
#define Guinea_Bissau_COUNTRY_STR "Guin�-Bissau"
#define Guyana_COUNTRY_STR "Guiana"
#define Haiti_COUNTRY_STR "Hait�"
#define Honduras_COUNTRY_STR "Honduras"
#define Hong_Kong_COUNTRY_STR "Hong Kong"
#define Hungary_COUNTRY_STR "Hungria"
#define Iceland_COUNTRY_STR "Isl�ndia"
#define India_COUNTRY_STR "�ndia"
#define Indonesia_COUNTRY_STR "Indon�sia"
#define INMARSAT_COUNTRY_STR "INMARSAT"
#define INMARSAT_Atlantic_East_COUNTRY_STR "INMARSAT Atl�ntico-leste"
#define Iran_COUNTRY_STR "Ir�"
#define Iraq_COUNTRY_STR "Iraque"
#define Ireland_COUNTRY_STR "Irlanda"
#define Israel_COUNTRY_STR "Israel"
#define Italy_COUNTRY_STR "It�lia"
#define Ivory_Coast_COUNTRY_STR "Costa do Marfim"
#define Japan_COUNTRY_STR "Jap�o"
#define Jordan_COUNTRY_STR "Jord�nia"
#define Kenya_COUNTRY_STR "Qu�nia"
#define South_Korea_COUNTRY_STR "Cor�ia do Sul"
#define Kuwait_COUNTRY_STR "Quait"
#define Liberia_COUNTRY_STR "Lib�ria"
#define Libya_COUNTRY_STR "L�bia"
#define Liechtenstein_COUNTRY_STR "Liechtenstein"
#define Luxembourg_COUNTRY_STR "Luxemburgo"
#define Malawi_COUNTRY_STR "Malawi"
#define Malaysia_COUNTRY_STR "Mal�sia"
#define Mali_COUNTRY_STR "Mali"
#define Malta_COUNTRY_STR "Malta"
#define Mexico_COUNTRY_STR "M�xico"
#define Monaco_COUNTRY_STR "M�naco"
#define Morocco_COUNTRY_STR "Marrocos"
#define Namibia_COUNTRY_STR "Nam�bia"
#define Nepal_COUNTRY_STR "Nepal"
#define Netherlands_COUNTRY_STR "Holanda"
#define Netherlands_Antilles_COUNTRY_STR "Antilhas Holandesas"
#define New_Caledonia_COUNTRY_STR "Nova Caled�nia"
#define New_Zealand_COUNTRY_STR "Nova Zel�ndia"
#define Nicaragua_COUNTRY_STR "Nicar�gua"
#define Nigeria_COUNTRY_STR "Nig�ria"
#define Norway_COUNTRY_STR "Noruega"
#define Oman_COUNTRY_STR "Om�"
#define Pakistan_COUNTRY_STR "Paquist�o"
#define Panama_COUNTRY_STR "Panam�"
#define Papua_New_Guinea_COUNTRY_STR "Papua Nova Guin�"
#define Paraguay_COUNTRY_STR "Paraguai"
#define Peru_COUNTRY_STR "Peru"
#define Philippines_COUNTRY_STR "Filipinas"
#define Poland_COUNTRY_STR "Pol�nia"
#define Portugal_COUNTRY_STR "Portugal"
#define Qatar_COUNTRY_STR "Qatar"
#define Romania_COUNTRY_STR "Rom�nia"
#define Russia_COUNTRY_STR "R�ssia"
#define Saipan_COUNTRY_STR "Saipan"
#define San_Marino_COUNTRY_STR "San Marino"
#define Saudia_Arabia_COUNTRY_STR "Ar�bia Saudita"
#define Saipan_COUNTRY_STR "Saipan"
#define Senegal_COUNTRY_STR "Senegal"
#define Singapore_COUNTRY_STR "Singapura"
#define Slovakia_COUNTRY_STR "Eslov�quia"
#define South_Africa_COUNTRY_STR "�frica do Sul"
#define Spain_COUNTRY_STR "Espanha"
#define Sri_Lanka_COUNTRY_STR "Sri Lanka"
#define Suriname_COUNTRY_STR "Suriname"
#define Sweden_COUNTRY_STR "Su�cia"
#define Switzerland_COUNTRY_STR "Su��a"
#define Taiwan_COUNTRY_STR "Taiwan"
#define Tanzania_COUNTRY_STR "Tanz�nia"
#define Thailand_COUNTRY_STR "Tail�ndia"
#define Tunisia_COUNTRY_STR "Tun�sia"
#define Turkey_COUNTRY_STR "Turquia"
#define United_Arab_Emirates_COUNTRY_STR "Emirados �rabes Unidos"
#define Uruguay_COUNTRY_STR "Uruguai"
#define UK_COUNTRY_STR "Reino Unido"
#define Ukraine_COUNTRY_STR "Ucr�nia"
#define Vatican_City_COUNTRY_STR "Cidade do Vaticano"
#define Venezuela_COUNTRY_STR "Venezuela"
#define Vietnam_COUNTRY_STR "Vietn�"
#define Yemen_COUNTRY_STR "Yemen"
#define Yugoslavia_COUNTRY_STR "Yugosl�via"
#define Zaire_COUNTRY_STR "Zaire"
#define Zimbabwe_COUNTRY_STR "Zimbabwe"
#define NON_COUNTRY_FUNNY_STR "Dimens�o X"
#define NON_COUNTRY_STR "Nao informado"

