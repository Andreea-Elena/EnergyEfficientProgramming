package com.example.javaapi.repositories;

import com.example.javaapi.db.Table1;
import jakarta.transaction.Transactional;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Modifying;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.util.List;

public interface Table1Repository extends JpaRepository<Table1, Integer> {

    @Query("SELECT t FROM Table1 t WHERE " +
            "(:columnA IS NULL OR t.columnA = :columnA) AND " +
            "(:columnB IS NULL OR t.columnB = :columnB) AND " +
            "(:columnC IS NULL OR t.columnC = :columnC) AND " +
            "(:columnD IS NULL OR t.columnD = :columnD)")
    List<Table1> findByColumns(
            @Param("columnA") String columnA,
            @Param("columnB") String columnB,
            @Param("columnC") String columnC,
            @Param("columnD") String columnD);

    @Transactional
    @Modifying
    @Query("DELETE FROM Table1 t WHERE " +
            "(:columnA IS NULL OR t.columnA = :columnA) AND " +
            "(:columnB IS NULL OR t.columnB = :columnB) AND " +
            "(:columnC IS NULL OR t.columnC = :columnC) AND " +
            "(:columnD IS NULL OR t.columnD = :columnD)")
    void deleteByColumns(
            @Param("columnA") String columnA,
            @Param("columnB") String columnB,
            @Param("columnC") String columnC,
            @Param("columnD") String columnD);

    @Transactional
    @Modifying
    @Query("UPDATE Table1 t SET t.columnC = :newColumnC, t.columnD = :newColumnD WHERE t.columnA = :columnA AND t.columnB = :columnB")
    void updateByColumnAAndColumnB(
            @Param("columnA") String columnA,
            @Param("columnB") String columnB,
            @Param("newColumnC") String newColumnC,
            @Param("newColumnD") String newColumnD);
}
