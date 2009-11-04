DROP TABLE IF EXISTS chan;
DROP TABLE IF EXISTS ison;
DROP TABLE IF EXISTS server;
DROP TABLE IF EXISTS user;
DROP TABLE IF EXISTS maxvalues;

CREATE TABLE chan (
  chanid int unsigned NOT NULL auto_increment,
  channel varchar(33) binary NOT NULL default '',
  topic text,
  topicauthor varchar(31) default NULL,
  topictime datetime default NULL,
  mode_lc enum('Y','N') NOT NULL default 'N',
  mode_lf enum('Y','N') NOT NULL default 'N',
  mode_li enum('Y','N') NOT NULL default 'N',
  mode_lj enum('Y','N') NOT NULL default 'N',
  mode_lk enum('Y','N') NOT NULL default 'N',
  mode_ll enum('Y','N') NOT NULL default 'N',
  mode_lm enum('Y','N') NOT NULL default 'N',
  mode_ln enum('Y','N') NOT NULL default 'N',
  mode_lp enum('Y','N') NOT NULL default 'N',
  mode_lr enum('Y','N') NOT NULL default 'N',
  mode_ls enum('Y','N') NOT NULL default 'N',
  mode_lt enum('Y','N') NOT NULL default 'N',
  mode_lu enum('Y','N') NOT NULL default 'N',
  mode_ly enum('Y','N') NOT NULL default 'N',
  mode_lz enum('Y','N') NOT NULL default 'N',
  mode_ua enum('Y','N') NOT NULL default 'N',
  mode_uc enum('Y','N') NOT NULL default 'N',
  mode_ud enum('Y','N') NOT NULL default 'N',
  mode_ug enum('Y','N') NOT NULL default 'N',
  mode_uh enum('Y','N') NOT NULL default 'N',
  mode_uk enum('Y','N') NOT NULL default 'N',
  mode_ul enum('Y','N') NOT NULL default 'N',
  mode_um enum('Y','N') NOT NULL default 'N',
  mode_un enum('Y','N') NOT NULL default 'N',
  mode_uo enum('Y','N') NOT NULL default 'N',
  mode_uq enum('Y','N') NOT NULL default 'N',
  mode_ur enum('Y','N') NOT NULL default 'N',
  mode_us enum('Y','N') NOT NULL default 'N',
  mode_ut enum('Y','N') NOT NULL default 'N',
  mode_uv enum('Y','N') NOT NULL default 'N',
  mode_lf_data varchar(20) NOT NULL default '',
  mode_lj_data varchar(5) NOT NULL default '',
  mode_lk_data varchar(23) NOT NULL default '',
  mode_ll_data int(10) NOT NULL default '0',
  mode_ul_data varchar(33) NOT NULL default '',
  PRIMARY KEY  (chanid),
  UNIQUE KEY channel (channel)
) TYPE=MyISAM;

CREATE TABLE ison (
  nickid int unsigned NOT NULL default '0',
  chanid int unsigned NOT NULL default '0',
  mode_la enum('Y','N') NOT NULL default 'N',
  mode_lh enum('Y','N') NOT NULL default 'N',
  mode_lo enum('Y','N') NOT NULL default 'N',
  mode_lq enum('Y','N') NOT NULL default 'N',
  mode_lv enum('Y','N') NOT NULL default 'N',
  PRIMARY KEY  (nickid,chanid),
  KEY nickid (nickid),
  KEY chanid (chanid)
) TYPE=MyISAM;

CREATE TABLE server (
  servid int unsigned NOT NULL auto_increment,
  server varchar(64) NOT NULL default '',
  comment varchar(255) NOT NULL default '',
  linkedto int unsigned default NULL,
  connecttime datetime default NULL,
  online enum('Y','N') NOT NULL DEFAULT 'Y',
  lastsplit datetime default NULL, 
  PRIMARY KEY  (servid),
  UNIQUE KEY server (server),
  KEY linkedto (linkedto)
) TYPE=MyISAM;


CREATE TABLE user (
  nickid int unsigned NOT NULL auto_increment,
  nick varchar(31) NOT NULL default '',
  realname varchar(51) NOT NULL default '',
  hostname varchar(64) NOT NULL default '',
  hiddenhostname varchar(64) NOT NULL default '',
  username varchar(11) NOT NULL default '',
  swhois varchar(32) NOT NULL default '',
  connecttime datetime NOT NULL default '0000-00-00 00:00:00',
  servid int unsigned NOT NULL default '0',
  away enum('Y','N') NOT NULL default 'N',
  awaymsg text,
  online enum('Y','N') NOT NULL DEFAULT 'Y',
  lastquit datetime default NULL,
  mode_la enum('Y','N') NOT NULL default 'N',
  mode_ld enum('Y','N') NOT NULL default 'N',
  mode_lg enum('Y','N') NOT NULL default 'N',
  mode_lh enum('Y','N') NOT NULL default 'N',
  mode_li enum('Y','N') NOT NULL default 'N',
  mode_lo enum('Y','N') NOT NULL default 'N',
  mode_lp enum('Y','N') NOT NULL default 'N',
  mode_lq enum('Y','N') NOT NULL default 'N',
  mode_lr enum('Y','N') NOT NULL default 'N',
  mode_lt enum('Y','N') NOT NULL default 'N',
  mode_lv enum('Y','N') NOT NULL default 'N',
  mode_lw enum('Y','N') NOT NULL default 'N',
  mode_lx enum('Y','N') NOT NULL default 'N',
  mode_lz enum('Y','N') NOT NULL default 'N',
  mode_ua enum('Y','N') NOT NULL default 'N',
  mode_ub enum('Y','N') NOT NULL default 'N',
  mode_uc enum('Y','N') NOT NULL default 'N',
  mode_ud enum('Y','N') NOT NULL default 'N',
  mode_ug enum('Y','N') NOT NULL default 'N',
  mode_uh enum('Y','N') NOT NULL default 'N',
  mode_un enum('Y','N') NOT NULL default 'N',
  mode_ur enum('Y','N') NOT NULL default 'N',
  mode_us enum('Y','N') NOT NULL default 'N',
  mode_ut enum('Y','N') NOT NULL default 'N',
  mode_uv enum('Y','N') NOT NULL default 'N',
  mode_uw enum('Y','N') NOT NULL default 'N',
  PRIMARY KEY  (nickid),
  UNIQUE KEY nick (nick),
  KEY servid (servid)
) TYPE=MyISAM;

CREATE TABLE maxvalues (
  type varchar(10) NOT NULL default '',
  val int unsigned NOT NULL default '0',
  time datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY(type)
) TYPE=MyISAM;
