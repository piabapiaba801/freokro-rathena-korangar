-- FreokRO V19: migrate persisted custom skills from the invalid 9000 range.
-- Mapping is one-to-one: 9000..9008 becomes 7000..7008.

START TRANSACTION;

UPDATE `skill` AS current_skill
INNER JOIN `skill` AS old_skill
  ON old_skill.`char_id` = current_skill.`char_id`
 AND old_skill.`id` = current_skill.`id` + 2000
SET current_skill.`lv` = GREATEST(current_skill.`lv`, old_skill.`lv`),
    current_skill.`flag` = GREATEST(current_skill.`flag`, old_skill.`flag`)
WHERE current_skill.`id` BETWEEN 7000 AND 7008;

INSERT IGNORE INTO `skill` (`char_id`, `id`, `lv`, `flag`)
SELECT `char_id`, `id` - 2000, `lv`, `flag`
FROM `skill`
WHERE `id` BETWEEN 9000 AND 9008;

DELETE FROM `skill`
WHERE `id` BETWEEN 9000 AND 9008;

COMMIT;
