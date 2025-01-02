DROP DATABASE IF EXISTS dsm_keynego;
CREATE DATABASE dsm_keynego;
USE dsm_keynego;

CREATE TABLE `clientkeys` (
  `UserID` varchar(128) NOT NULL,
  `PublicKey` text,
  `KeyLength` double DEFAULT NULL,
  `CreateTime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `UpdateTime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`UserID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;


CREATE TABLE `serverkeys` (
  `ServerID` varchar(128) NOT NULL,
  `PublicKey` text,
  `KeyLength` double DEFAULT NULL,
  `CreateTime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `UpdateTime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`ServerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE `sessionkeys` (
  `SessionID` varchar(128) NOT NULL,
  `UserID` varchar(128) NOT NULL,
  `ServerID` varchar(128) NOT NULL,
  `EncryptedKeyForClient` text,
  `EncryptedKeyForServer` text,
  `KeyCreateTime` timestamp NULL DEFAULT NULL ON UPDATE CURRENT_TIMESTAMP,
  `KeyExpireTime` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`SessionID`),
  KEY `fk_clientID` (`UserID`),
  KEY `fk_serverID` (`ServerID`),
  CONSTRAINT `fk_clientID` FOREIGN KEY (`UserID`) REFERENCES `clientkeys` (`UserID`),
  CONSTRAINT `fk_serverID` FOREIGN KEY (`ServerID`) REFERENCES `serverkeys` (`ServerID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;