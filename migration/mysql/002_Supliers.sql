CREATE TABLE supliers (
	`id` INT NOT NULL AUTO_INCREMENT,
    `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updated_at` TIMESTAMP NULL,
    `deleted_at` TIMESTAMP NULL,
    `name` VARCHAR(64) NOT NULL,
    `code` VARCHAR(16) NOT NULL,
    `address` VARCHAR(255) NOT NULL DEFAULT '',
    `phone` VARCHAR(64) NOT NULL DEFAULT '',
    UNIQUE INDEX `NAME` (`name` ASC),
    PRIMARY KEY (`id`)
) ENGINE = InnoDB;

INSERT INTO supliers (name, code, address, phone) VALUES ('CV. Sultan Food', 'TF', 'Jogonalan Lor RT 2', '08123456789,085235419949');